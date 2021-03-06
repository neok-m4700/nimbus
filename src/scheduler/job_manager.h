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
  * Scheduler Job Manager object. This module serves the scheduler by providing
  * facilities about jobs ready to be maped, and their dependencies.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#ifndef NIMBUS_SRC_SCHEDULER_JOB_MANAGER_H_
#define NIMBUS_SRC_SCHEDULER_JOB_MANAGER_H_

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <iostream> // NOLINT
#include <fstream> // NOLINT
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <map>
#include <set>
#include "src/shared/nimbus_types.h"
#include "src/shared/dbg.h"
#include "src/shared/graph.h"
#include "src/shared/id_maker.h"
#include "src/scheduler/job_entry.h"
#include "src/scheduler/complex_job_entry.h"
#include "src/scheduler/after_map.h"
#include "src/shared/logical_data_object.h"
#include "src/scheduler/version_manager.h"
#include "src/scheduler/checkpoint_manager.h"
#include "src/scheduler/physical_data.h"
#include "src/shared/log.h"

namespace nimbus {
class JobManager {
  public:
    explicit JobManager();
    virtual ~JobManager();

    void set_after_map(AfterMap* after_map);
    void set_ldo_map_p(const LdoMap* ldo_map_p);
    void set_binding_memoization_active(bool flag);
    void set_cascaded_binding_active(bool flag);
    void set_fault_tolerance_active(bool flag);
    void set_checkpoint_creation_period(int64_t period);

    Graph<JobEntry, job_id_t> *job_graph_p();

    JobEntry* AddComputeJobEntry(const std::string& job_name,
                                 const job_id_t& job_id,
                                 const IDSet<logical_data_id_t>& read_set,
                                 const IDSet<logical_data_id_t>& write_set,
                                 const IDSet<logical_data_id_t>& scratch_set,
                                 const IDSet<logical_data_id_t>& reduce_set,
                                 const IDSet<job_id_t>& before_set,
                                 const IDSet<job_id_t>& after_set,
                                 const job_id_t& parent_job_id,
                                 const job_id_t& future_job_id,
                                 const bool& sterile,
                                 const GeometricRegion& region,
                                 const Parameter& params,
                                 const CombinerMap& combiner_map);

    bool AddComplexJobEntry(ComplexJobEntry* complex_job);

    JobEntry* AddExplicitCopyJobEntry();

    JobEntry* AddKernelJobEntry();

    JobEntry* AddMainJobEntry(const job_id_t& job_id);

    JobEntry* AddCreateDataJobEntry(const job_id_t& job_id);

    JobEntry* AddLocalCopyJobEntry(const job_id_t& job_id);

    JobEntry* AddRemoteCopySendJobEntry(const job_id_t& job_id);

    JobEntry* AddRemoteCopyReceiveJobEntry(const job_id_t& job_id);

    JobEntry* AddFutureJobEntry(const job_id_t& job_id);

    bool RemoveJobEntry(JobEntry* job);

    size_t NumJobsReadyToAssign();

    size_t GetJobsReadyToAssign(JobEntryList* list,
                                size_t max_num,
                                load_balancing_id_t lb_id,
                                bool *safe_to_load_balance_);

    size_t RemoveObsoleteJobEntries(size_t max_to_remove);

    void NotifyJobDone(job_id_t job_id);

    void NotifyJobAssignment(JobEntry *job);

    void DefineData(job_id_t job_id, logical_data_id_t ldid);

    bool ResolveJobDataVersions(JobEntry *job);

    bool ResolveEntireContextForJob(JobEntry *job);

    bool GetBaseVersionMapFromJob(job_id_t job_id,
                                  boost::shared_ptr<VersionMap>& vmap_base);

    bool ResolveJobDataVersionsForPattern(JobEntry *job,
                  const BindingTemplate::PatternList* patterns);

    bool ResolveJobDataVersionsForSingleEntry(JobEntry *job,
                  const logical_data_id_t& ldid,
                  data_version_t *version);

    bool MemoizeVersionsForTemplate(JobEntry *job);

    size_t GetJobsNeedDataVersion(JobEntryList* list,
                                  VersionedLogicalData vld);

    bool CompleteJobForCheckpoint(checkpoint_id_t checkpoint_id,
                                  const JobEntry *job);

    bool AddSaveDataJobToCheckpoint(checkpoint_id_t checkpoint_id,
                                    job_id_t job_id,
                                    logical_data_id_t ldid,
                                    data_version_t version,
                                    worker_id_t worker_id);

    bool NotifySaveDataJobDoneForCheckpoint(checkpoint_id_t checkpoint_id,
                                            job_id_t job_id,
                                            std::string handle);

    bool GetHandleToLoadData(checkpoint_id_t checkpoint_id,
                             logical_data_id_t ldid,
                             data_version_t version,
                             WorkerHandleList *handles);

    bool RewindFromLastCheckpoint(checkpoint_id_t *checkpoint_id);

    bool AllJobsAreDone();

    void UpdateJobBeforeSet(JobEntry* job);

    void UpdateBeforeSet(IDSet<job_id_t>* before_set);

    bool CausingUnwantedSerialization(JobEntry* job,
                                      const logical_data_id_t& l_id,
                                      const PhysicalData& pd,
                                      bool memoizing_mode = false);

  private:
    AfterMap *after_map_;
    const LdoMap *ldo_map_p_;
    VersionManager version_manager_;
    CheckpointManager checkpoint_manager_;
    bool binding_memoization_active_;
    bool cascaded_binding_active_;

    Graph<JobEntry, job_id_t> job_graph_;
    ComplexJobEntryMap complex_jobs_;
    boost::mutex job_graph_mutex_;

    JobEntryList jobs_done_;
    boost::mutex jobs_done_mutex_;

    JobEntryMap non_sterile_jobs_;
    bool fault_tolerance_active_;
    int64_t checkpoint_creation_period_;
    int64_t last_checkpoint_time_;
    counter_t non_sterile_counter_;

    JobEntryMap jobs_ready_to_assign_;
    JobEntryMap jobs_pending_to_assign_;
    boost::recursive_mutex job_queue_mutex_;

    bool AddJobEntryToJobGraph(job_id_t job_id, JobEntry *job);

    bool GetJobEntryFromJobGraph(job_id_t job_id, JobEntry*& job);


    bool GetComplexJobContainer(const job_id_t& job_id,
                                ComplexJobEntry*& complex_job);

    bool RemoveJobEntryFromJobGraph(job_id_t job_id);

    bool ClearJobGraph();

    Edge<JobEntry, job_id_t>* AddEdgeToJobGraph(job_id_t from, job_id_t to);

    bool GetJobGraphVertex(job_id_t job_id, Vertex<JobEntry, job_id_t> **vertex);

    bool AddJobEntryIncomingEdges(JobEntry *job);

    void ReceiveMetaBeforeSetDepthVersioningDependency(JobEntry* job);

    void PassMetaBeforeSetDepthVersioningDependency(JobEntry* job);
};

}  // namespace nimbus
#endif  // NIMBUS_SRC_SCHEDULER_JOB_MANAGER_H_
