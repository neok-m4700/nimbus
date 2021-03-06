/* Copyright 2013 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 vd* are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Author: Hang Qu <quhang@stanford.edu>
 * Modifier for smoke: Andrew Lim <alim16@stanford.edu>
 */

#include <sstream>
#include <string>
#include <vector>

#include <PhysBAM_Tools/Krylov_Solvers/PCG_SPARSE.h>

#include "applications/physbam/smoke/app_utils.h"
#include "applications/physbam/smoke/data_names.h"
#include "applications/physbam/smoke/job_names.h"
#include "applications/physbam/smoke/physbam_utils.h"
#include "applications/physbam/smoke/projection/projection_driver.h"
#include "applications/physbam/smoke/smoke_driver.h"
#include "applications/physbam/smoke/smoke_example.h"
#include "src/shared/dbg.h"
#include "src/shared/nimbus.h"
// #include "worker/job_query.h"

#include "applications/physbam/smoke/projection/job_projection_loop_iteration.h"

namespace application {

JobProjectionLoopIteration::JobProjectionLoopIteration(
    nimbus::Application *app) {
  set_application(app);
}

nimbus::Job* JobProjectionLoopIteration::Clone() {
  return new JobProjectionLoopIteration(application());
}

void JobProjectionLoopIteration::Execute(
    nimbus::Parameter params,
    const nimbus::DataArray& da) {
  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  dbg(APP_LOG, "Executing PROJECTION_LOOP_ITERATION job.\n");

  InitConfig init_config;
  T dt;
  int iteration;
  std::string params_str(params.ser_data().data_ptr_raw(),
                         params.ser_data().size());
  LoadParameter(params_str, &init_config.frame, &init_config.time, &dt,
                &init_config.global_region, &init_config.local_region,
                &iteration);
  const nimbus::GeometricRegion& global_region = init_config.global_region;
  const int& frame = init_config.frame;
  const T& time = init_config.time;

  DataConfig data_config;
  // TODO(quhang), LOCAL_N and INTERIOR_N might not be needed.
  data_config.SetFlag(DataConfig::PROJECTION_LOCAL_N);
  data_config.SetFlag(DataConfig::PROJECTION_INTERIOR_N);
  data_config.SetFlag(DataConfig::PROJECTION_LOCAL_RESIDUAL);
  data_config.SetFlag(DataConfig::PROJECTION_GLOBAL_TOLERANCE);
  data_config.SetFlag(DataConfig::PROJECTION_DESIRED_ITERATIONS);

  PhysBAM::PCG_SPARSE<float> pcg_temp;
  pcg_temp.Set_Maximum_Iterations(application::kMaxIterations);
  pcg_temp.evolution_solver_type = PhysBAM::krylov_solver_cg;
  pcg_temp.cg_restart_iterations = 40;

  PhysBAM::ProjectionDriver projection_driver(
      pcg_temp, init_config, data_config);

  dbg(APP_LOG, "Job PROJECTION_LOOP_ITERATION starts (dt=%f).\n", dt);

  {
    application::ScopeTimer scope_timer(name() + "-load");
    projection_driver.LoadFromNimbus(this, da);
  }

  projection_driver.projection_data.residual =
      projection_driver.projection_data.local_residual;
  projection_driver.projection_data.iteration = iteration;

  dbg(APP_LOG, "[CONTROL FLOW] size = %d, Iteration = %d, "
      "Desired iteration = %d, "
      "Residual = %f, Global tolerance = %f\n",
      projection_driver.projection_data.interior_n,
      projection_driver.projection_data.iteration,
      projection_driver.projection_data.desired_iterations,
      projection_driver.projection_data.residual,
      projection_driver.projection_data.global_tolerance);
  // Decides whether to spawn a new projection loop or finish it.
  if (projection_driver.projection_data.residual <=
      projection_driver.projection_data.global_tolerance ||
      projection_driver.projection_data.iteration >=
      projection_driver.projection_data.desired_iterations) {
    // Finishes projection iterating.
    StartTemplate("projection_loop_iteration_end");

    int projection_job_num = 2;
    std::vector<nimbus::job_id_t> projection_job_ids;
    GetNewJobID(&projection_job_ids, projection_job_num);

    int trans_job_num = kProjAppPartNum;
    std::vector<nimbus::job_id_t> trans_job_ids;
    GetNewJobID(&trans_job_ids, trans_job_num);

    int wrapup_job_num = kProjAppPartNum;
    std::vector<nimbus::job_id_t> wrapup_job_ids;
    GetNewJobID(&wrapup_job_ids, wrapup_job_num);

    nimbus::IDSet<nimbus::logical_data_id_t> read, write;
    nimbus::IDSet<nimbus::job_id_t> before, after;

    std::vector<nimbus::Parameter> default_part_params;
    default_part_params.resize(kProjAppPartNum);
    for (size_t i = 0; i < kProjAppPartNum; ++i) {
      std::string default_params_str;
      SerializeParameter(
          frame, time, dt, global_region, ph.map()["kProjRegY2W0Central"][i],
          &default_params_str);
      default_part_params[i].set_ser_data(SerializedData(default_params_str));
    }

    // Projection transform_pressure.
    for (int index = 0; index < trans_job_num; ++index) {
      read.clear();
      LoadLdoIdsInSet( &read, ph.map()["kProjRegY2W3Central"][index],
                          APP_PROJECTION_LOCAL_N, APP_PROJECTION_INTERIOR_N,
                          APP_VECTOR_PRESSURE, APP_INDEX_M2C, NULL);
      write.clear();
      LoadLdoIdsInSet( &write, ph.map()["kProjRegY2W3Central"][index],
                          APP_PRESSURE, NULL);

      after.clear();
      before.clear();
      StageJobAndLoadBeforeSet(&before, PROJECTION_TRANSFORM_PRESSURE,
                               trans_job_ids[index],
                               read, write);
      SpawnComputeJob(PROJECTION_TRANSFORM_PRESSURE,
                      trans_job_ids[index],
                      read, write, before, after,
                      default_part_params[index], true,
                      ph.map()["kProjRegY2W3Central"][index]);
    }

    MarkEndOfStage();

    // Projection wrapup.
    for (int index = 0; index < wrapup_job_num; ++index) {
      read.clear();
      LoadLdoIdsInSet( &read, ph.map()["kProjRegY2W3Outer"][index], APP_FACE_VEL,
                          NULL);
      LoadLdoIdsInSet( &read, ph.map()["kProjRegY2W1Outer"][index], APP_PSI_D, APP_PSI_N,
                          APP_PRESSURE, NULL);
      LoadLdoIdsInSet( &read, ph.map()["kProjRegY2W1Central"][index], APP_U_INTERFACE, NULL);
      write.clear();
      LoadLdoIdsInSet( &write, ph.map()["kProjRegY2W0Central"][index], APP_FACE_VEL, NULL);
      LoadLdoIdsInSet( &write, ph.map()["kProjRegY2W1CentralWGB"][index], APP_PRESSURE, NULL);

      after.clear();
      before.clear();
      StageJobAndLoadBeforeSet(&before, PROJECTION_WRAPUP,
                               wrapup_job_ids[index],
                               read, write);
      SpawnComputeJob(PROJECTION_WRAPUP,
                      wrapup_job_ids[index],
                      read, write, before, after,
                      default_part_params[index], true,
                      ph.map()["kProjRegY2W3Central"][index]);
    }

    MarkEndOfStage();

    // Loop iteration part two.
    read.clear();
    write.clear();
    nimbus::Parameter loop_iteration_part_two_params;
    std::string loop_iteration_part_two_str;
    SerializeParameter(frame, time, dt, global_region, global_region,
                       &loop_iteration_part_two_str);
    loop_iteration_part_two_params.set_ser_data(
        SerializedData(loop_iteration_part_two_str));


    after.clear();
    before.clear();
    StageJobAndLoadBeforeSet(&before, LOOP_ITERATION_PART_TWO,
                             projection_job_ids[1],
                             read, write,
                             true);
    SpawnComputeJob(LOOP_ITERATION_PART_TWO,
                    projection_job_ids[1],
                    read, write, before, after,
                    loop_iteration_part_two_params, false,
                    ph.map()["kRegW3Central"][0]);

    MarkEndOfStage();

    EndTemplate("projection_loop_iteration_end");

    // if (time == 0) {
    //   dbg(APP_LOG, "Print job dependency figure.\n");
    //   job_query.GenerateDotFigure("projection_iteration_last.dot");
    // }
  } else {
    // Spawns a new projection iteration.
    StartTemplate("projection_loop_iteration");

    nimbus::Parameter default_params;
    std::string default_params_str;
    SerializeParameter(frame, time, dt, global_region, global_region, iteration,
                       &default_params_str);
    default_params.set_ser_data(SerializedData(default_params_str));

    std::vector<nimbus::Parameter> default_part_params;
    default_part_params.resize(kProjAppPartNum);
    for (size_t i = 0; i < kProjAppPartNum; ++i) {
      std::string default_params_str;
      SerializeParameter(
          frame, time, dt, global_region, ph.map()["kProjRegY2W0Central"][i], iteration,
          &default_params_str);
      default_part_params[i].set_ser_data(SerializedData(default_params_str));
    }

    int projection_job_num = 7;
    std::vector<nimbus::job_id_t> projection_job_ids;
    GetNewJobID(&projection_job_ids, projection_job_num);

    int step_one_job_num = kProjAppPartNum;
    std::vector<nimbus::job_id_t> step_one_job_ids;
    GetNewJobID(&step_one_job_ids, step_one_job_num);

    int step_two_job_num = kProjAppPartNum;
    std::vector<nimbus::job_id_t> step_two_job_ids;
    GetNewJobID(&step_two_job_ids, step_two_job_num);

    int step_three_job_num = kProjAppPartNum;
    std::vector<nimbus::job_id_t> step_three_job_ids;
    GetNewJobID(&step_three_job_ids, step_three_job_num);

    int step_four_job_num = kProjAppPartNum;
    std::vector<nimbus::job_id_t> step_four_job_ids;
    GetNewJobID(&step_four_job_ids, step_four_job_num);

    nimbus::IDSet<nimbus::logical_data_id_t> read, write;
    nimbus::IDSet<nimbus::job_id_t> before, after;

    // STEP_ONE.
    for (int index = 0; index < step_one_job_num; ++index) {
      read.clear();
      LoadLdoIdsInSet(
          &read, ph.map()["kProjRegY2W0Central"][index],
          APP_VECTOR_TEMP,
          APP_PROJECTION_LOCAL_N, APP_PROJECTION_INTERIOR_N,
          APP_MATRIX_C, APP_VECTOR_B, APP_VECTOR_Z, NULL);
      write.clear();
      LoadLdoIdsInSet(
          &write, ph.map()["kProjRegY2W0Central"][index],
          APP_VECTOR_TEMP,
          APP_VECTOR_Z, APP_PROJECTION_LOCAL_RHO, NULL);

      after.clear();
      before.clear();
      StageJobAndLoadBeforeSet(&before, PROJECTION_STEP_ONE,
                               step_one_job_ids[index],
                               read, write);
      SpawnComputeJob(PROJECTION_STEP_ONE,
                      step_one_job_ids[index],
                      read, write, before, after,
                      default_part_params[index], true,
                      ph.map()["kProjRegY2W3Central"][index]);
    }

    MarkEndOfStage();

    // REDUCE_RHO
    read.clear();
    LoadLdoIdsInSet(
        &read, ph.map()["kRegW0Central"][0], APP_PROJECTION_LOCAL_N,
        APP_PROJECTION_INTERIOR_N, APP_PROJECTION_LOCAL_RHO,
        APP_PROJECTION_GLOBAL_RHO, NULL);
    write.clear();
    LoadLdoIdsInSet(
        &write, ph.map()["kRegW0Central"][0], APP_PROJECTION_LOCAL_RHO,
        APP_PROJECTION_GLOBAL_RHO, APP_PROJECTION_GLOBAL_RHO_OLD,
        APP_PROJECTION_BETA, NULL);


    after.clear();
    before.clear();
    StageJobAndLoadBeforeSet(&before, PROJECTION_REDUCE_RHO,
                             projection_job_ids[1],
                             read, write);
    SpawnComputeJob(PROJECTION_REDUCE_RHO,
                    projection_job_ids[1],
                    read, write, before, after,
                    default_params, true,
                    ph.map()["kRegW3Central"][0]);
 
    MarkEndOfStage();

    // STEP_TWO
    for (int index = 0; index < step_two_job_num; ++index) {
      read.clear();
      LoadLdoIdsInSet(
          &read, ph.map()["kRegW0Central"][0], APP_PROJECTION_BETA, NULL);
      LoadLdoIdsInSet(
          &read, ph.map()["kProjRegY2W0Central"][index],
          APP_PROJECTION_LOCAL_N, APP_PROJECTION_INTERIOR_N,
          APP_VECTOR_Z,
          APP_VECTOR_P_META_FORMAT, APP_INDEX_C2M,
          NULL);
      write.clear();
      LoadLdoIdsInSet( &write, ph.map()["kProjRegY2W0Central"][index],
                          APP_VECTOR_P_META_FORMAT,
                          NULL);

      after.clear();
      before.clear();
      StageJobAndLoadBeforeSet(&before, PROJECTION_STEP_TWO,
                               step_two_job_ids[index],
                               read, write);
      SpawnComputeJob(PROJECTION_STEP_TWO,
                      step_two_job_ids[index],
                      read, write, before, after,
                      default_part_params[index], true,
                      ph.map()["kProjRegY2W3Central"][index]);
    }

    MarkEndOfStage();

    // STEP_THREE
    for (int index = 0; index < step_three_job_num; ++index) {
      read.clear();
      LoadLdoIdsInSet( &read, ph.map()["kProjRegY2W1Outer"][index],
                          APP_VECTOR_P_META_FORMAT,
                          NULL);
      LoadLdoIdsInSet(
          &read, ph.map()["kProjRegY2W0Central"][index],
          APP_VECTOR_TEMP,
          APP_PROJECTION_LOCAL_N, APP_PROJECTION_INTERIOR_N,
          APP_MATRIX_A,
          APP_INDEX_C2M,
          NULL);
      write.clear();
      LoadLdoIdsInSet(
          &write, ph.map()["kProjRegY2W0Central"][index],
          APP_VECTOR_TEMP,
          APP_PROJECTION_LOCAL_DOT_PRODUCT_FOR_ALPHA, NULL);

      after.clear();
      before.clear();
      StageJobAndLoadBeforeSet(&before, PROJECTION_STEP_THREE,
                               step_three_job_ids[index],
                               read, write);
      SpawnComputeJob(PROJECTION_STEP_THREE,
                      step_three_job_ids[index],
                      read, write, before, after,
                      default_part_params[index], true,
                      ph.map()["kProjRegY2W3Central"][index]);
    }

    MarkEndOfStage();

    // REDUCE_ALPHA
    read.clear();
    LoadLdoIdsInSet(
        &read, ph.map()["kRegW0Central"][0], APP_PROJECTION_LOCAL_N,
        APP_PROJECTION_INTERIOR_N, APP_PROJECTION_LOCAL_DOT_PRODUCT_FOR_ALPHA,
        NULL);
    LoadLdoIdsInSet(
        &read, ph.map()["kRegW0Central"][0], APP_PROJECTION_GLOBAL_RHO, NULL);
    write.clear();
    LoadLdoIdsInSet(
        &write, ph.map()["kRegW0Central"][0], APP_PROJECTION_ALPHA, NULL);


    after.clear();
    before.clear();
    StageJobAndLoadBeforeSet(&before, PROJECTION_REDUCE_ALPHA,
                             projection_job_ids[4],
                             read, write);
    SpawnComputeJob(PROJECTION_REDUCE_ALPHA,
                    projection_job_ids[4],
                    read, write, before, after,
                    default_params, true,
                    ph.map()["kRegW3Central"][0]);
 
    MarkEndOfStage();

    // STEP_FOUR
    // Only interior p is needed.
    for (int index = 0; index < step_four_job_num; ++index) {
      read.clear();
      LoadLdoIdsInSet(
          &read, ph.map()["kProjRegY2W0Central"][index],
          APP_PROJECTION_LOCAL_N, APP_PROJECTION_INTERIOR_N,
          APP_VECTOR_PRESSURE,
          APP_VECTOR_P_META_FORMAT, APP_INDEX_C2M,
          APP_VECTOR_TEMP, APP_VECTOR_B, NULL);
      LoadLdoIdsInSet(
          &read, ph.map()["kRegW0Central"][0], APP_PROJECTION_ALPHA, NULL);
      write.clear();
      LoadLdoIdsInSet(
          &write, ph.map()["kProjRegY2W0Central"][index], APP_VECTOR_B,
          APP_PROJECTION_LOCAL_RESIDUAL, APP_VECTOR_PRESSURE, NULL);


      after.clear();
      before.clear();
      StageJobAndLoadBeforeSet(&before, PROJECTION_STEP_FOUR,
                               step_four_job_ids[index],
                               read, write);
      SpawnComputeJob(PROJECTION_STEP_FOUR,
                      step_four_job_ids[index],
                      read, write, before, after,
                      default_part_params[index], true,
                      ph.map()["kProjRegY2W3Central"][index]);
    }

    MarkEndOfStage();

    // NEXT_ITERATION
    // TODO(quhang), needs a clean way to deal with LOCAL_N and INTERIOR_N here.
    read.clear();
    LoadLdoIdsInSet(
        &read, ph.map()["kRegW0Central"][0], APP_PROJECTION_LOCAL_RESIDUAL,
        APP_PROJECTION_INTERIOR_N,
        APP_PROJECTION_GLOBAL_TOLERANCE, APP_PROJECTION_DESIRED_ITERATIONS,
        NULL);
    write.clear();
    nimbus::Parameter next_iteration_params;
    std::string next_iteration_params_str;
    SerializeParameter(frame, time, dt, global_region, global_region,
                       iteration + 1,
                       &next_iteration_params_str);
    next_iteration_params.set_ser_data(
        SerializedData(next_iteration_params_str));


    after.clear();
    before.clear();
    StageJobAndLoadBeforeSet(&before, PROJECTION_LOOP_ITERATION,
                             projection_job_ids[6],
                             read, write,
                             true);
    SpawnComputeJob(PROJECTION_LOOP_ITERATION,
                    projection_job_ids[6],
                    read, write, before, after,
                    next_iteration_params, false,
                    ph.map()["kRegW3Central"][0]);

    MarkEndOfStage();

    // if (time == 0 && iteration == 1) {
    //   dbg(APP_LOG, "Print job dependency figure.\n");
    //   job_query.GenerateDotFigure("projection_iteration_first.dot");
    // }

    EndTemplate("projection_loop_iteration");
  }

  // TODO(quhang), removes the saving if possible.
  {
    application::ScopeTimer scope_timer(name() + "-save");
    projection_driver.SaveToNimbus(this, da);
  }


  // dbg(APP_LOG, "Completed executing PROJECTION_LOOP_ITERATION job\n");
  // {
  //   struct timeval t;
  //   gettimeofday(&t, NULL);
  //   double time  = (static_cast<double>(t.tv_sec - start_time.tv_sec)) +
  //       .000001 * (static_cast<double>(t.tv_usec - start_time.tv_usec));
  //   dbg(APP_LOG, "\nThe query time spent in job PROJECTION_LOOP_ITERATION is %f seconds.\n",
  //       time);
  // }
}

}  // namespace application
