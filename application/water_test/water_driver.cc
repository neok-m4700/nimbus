/*
 * Copyright 2013 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
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
  * Author: Chinmayee Shah <chinmayee.shah@stanford.edu>
  */

#include <iostream>
#include "./water_driver.h"
#include "./water_data_types.h"

using namespace PhysBAM;

template <class TV> WaterDriver<TV> ::
WaterDriver(const STREAM_TYPE stream_type_input):
    stream_type(stream_type_input)
{
    // setup time
    initial_time = (T)0;
    first_frame = (T)0;
    time = (T)0;

    last_frame = 100;
    frame_rate = 24;
    current_frame = 0;

    // other parameters
    number_of_ghost_cells = 3;
    cfl = 0.9;

    // I/O & logging
    write_substeps_level = -1;
    write_output_files = true;
    output_directory = "output";

    // debugging information
    id_debug = driver_id;
};

template<class TV>
void Write_Substep_Helper
(void *writer, const std::string &title, int substep, int level)
{
    ((WaterDriver<TV> *)writer)->Write_Substep(title, substep, level);
};

template <class TV>
Data* WaterDriver<TV> :: clone()
{
    std::cout << "Cloning waterdriver\n";
    return new WaterDriver<TV>(stream_type);
};

template <class TV>
void WaterDriver<TV> :: create()
{
    std::cout << "Initialize water driver ...\n";

    FILE_UTILITIES::Create_Directory(output_directory+"/common");
    LOG::Instance()->Copy_Log_To_File
        (output_directory+"/common/log.txt", false);

    Initialize_Particles();
    Initialize_Read_Write_General_Structures();

    DEBUG_SUBSTEPS::Set_Substep_Writer((void *)this, &Write_Substep_Helper<TV>);

    std::cout << "Completed initializing water driver\n";
}

template <class TV>
int WaterDriver<TV> :: get_debug_info()
{
    return id_debug;
}

template <class TV> void WaterDriver<TV>::
Get_Levelset_Velocity(
        const GRID<TV> &grid,
        T_LEVELSET& levelset,
        ARRAY<T,FACE_INDEX<TV::dimension> > &V_levelset,
        const T time) const PHYSBAM_OVERRIDE
{
    FaceArray<TV> *fv = (FaceArray<TV> *)face_velocities;
    V_levelset = *fv->data;
}

template <class TV> void WaterDriver<TV>::
Adjust_Particle_For_Domain_Boundaries(
        PARTICLE_LEVELSET_PARTICLES<TV> &particles,
        const int index, TV &V,
        const PARTICLE_LEVELSET_PARTICLE_TYPE particle_type,
        const T dt, const T time)
{
    NonAdvData<TV, T> *sd = (NonAdvData<TV, T> *)sim_data;
    sd->Adjust_Particle_For_Domain_Boundaries(particles, index, V,
            particle_type, dt, time);
}

#ifndef TEMPLATE_USE
#define TEMPLATE_USE
typedef VECTOR<float, 2> TVF2;
typedef float TF;
#endif  // TEMPLATE_USE

template class WaterDriver<TVF2>;
