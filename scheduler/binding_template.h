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
  * This is BindingTemplate module to hold and instantiate copy and compute
  * commands sent to workers to bind physical data to a batch of jobs in a
  * template complex job.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#ifndef NIMBUS_SCHEDULER_BINDING_TEMPLATE_H_
#define NIMBUS_SCHEDULER_BINDING_TEMPLATE_H_

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <iostream> // NOLINT
#include <fstream> // NOLINT
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <utility>
#include <algorithm>
#include <map>
#include <set>
#include "shared/nimbus_types.h"
#include "shared/scheduler_server.h"
#include "shared/scheduler_command_include.h"
#include "shared/dbg.h"
#include "shared/log.h"

namespace nimbus {

class TemplateEntry;

class BindingTemplate {
  public:
    BindingTemplate(const std::string& record_name,
                    const std::vector<job_id_t>& compute_job_ids,
                    TemplateEntry* template_entry,
                    bool worker_template_active);

    ~BindingTemplate();


    bool finalized() const;
    size_t copy_job_num() const;
    size_t compute_job_num() const;
    std::string record_name() const;

    bool Finalize(const std::vector<job_id_t>& compute_job_ids);

    bool Instantiate(const std::vector<job_id_t>& compute_job_ids,
                     const std::vector<Parameter>& parameters,
                     const std::vector<job_id_t>& copy_job_ids,
                     const std::vector<physical_data_id_t> *physical_ids,
                     SchedulerServer *server);

    enum VERSION_TYPE {
      REGULAR,
      WILD_CARD
    };

    class PatternEntry {
      public:
        PatternEntry(const worker_id_t& worker_id,
                     const logical_data_id_t& ldid,
                     const physical_data_id_t& pdid,
                     VERSION_TYPE version_type,
                     data_version_t version_diff_from_base)
          : worker_id_(worker_id),
            ldid_(ldid),
            pdid_(pdid),
            version_type_(version_type),
            version_diff_from_base_(version_diff_from_base) {}

        ~PatternEntry();

        worker_id_t worker_id_;
        logical_data_id_t ldid_;
        physical_data_id_t pdid_;
        VERSION_TYPE version_type_;
        data_version_t version_diff_from_base_;
    };

    typedef std::vector<PatternEntry*> PatternList;
    typedef boost::unordered_map<physical_data_id_t, PatternEntry*> PatternMap;
    typedef std::pair<PatternList*, PatternList*> PatternBucket;
    typedef boost::unordered_map<logical_data_id_t, PatternBucket> PatternSorted;
    typedef std::vector<std::pair<size_t, size_t> > PatternMetaData;

    const PatternMetaData* patterns_meta_data_p() const;
    const PatternList* entry_pattern_list_p() const;
    const PatternList* end_pattern_list_p() const;

    bool TrackDataObject(const worker_id_t& worker_id,
                         const logical_data_id_t& ldid,
                         const physical_data_id_t& pdid,
                         VERSION_TYPE version_type,
                         data_version_t version_diff_from_base);

    bool UpdateDataObject(const physical_data_id_t& pdid,
                          data_version_t version_diff_from_base);

    bool AddComputeJobCommand(ComputeJobCommand* command,
                              worker_id_t w_id);

    bool AddLocalCopyCommand(LocalCopyCommand* command,
                             worker_id_t w_id);

    bool AddRemoteCopySendCommand(RemoteCopySendCommand* command,
                                  worker_id_t w_id);

    bool AddRemoteCopyReceiveCommand(RemoteCopyReceiveCommand* command,
                                     worker_id_t w_id);

  private:
    typedef boost::shared_ptr<job_id_t> JobIdPtr;
    typedef std::vector<JobIdPtr> JobIdPtrList;
    typedef boost::unordered_set<JobIdPtr> JobIdPtrSet;
    typedef boost::unordered_map<job_id_t, JobIdPtr> JobIdPtrMap;

    typedef boost::shared_ptr<physical_data_id_t> PhyIdPtr;
    typedef std::vector<PhyIdPtr> PhyIdPtrList;
    typedef boost::unordered_set<PhyIdPtr> PhyIdPtrSet;
    typedef boost::unordered_map<job_id_t, PhyIdPtr> PhyIdPtrMap;

    enum CommandTemplateType {
      BASE,
      COMPUTE,
      LC,
      RCS,
      RCR
    };

    class CommandTemplate {
      public:
        CommandTemplate() {type_ = BASE;}
        ~CommandTemplate() {}

        CommandTemplateType type_;
    };

    typedef std::vector<CommandTemplate*> CommandTemplateVector;

    class ComputeJobCommandTemplate : public CommandTemplate {
      public:
        ComputeJobCommandTemplate(const std::string& job_name,
                                  JobIdPtr job_id_ptr,
                                  const PhyIdPtrSet& read_set_ptr,
                                  const PhyIdPtrSet& write_set_ptr,
                                  const JobIdPtrSet& before_set_ptr,
                                  const JobIdPtrSet& after_set_ptr,
                                  JobIdPtr future_job_id_ptr,
                                  const bool& sterile,
                                  const GeometricRegion& region,
                                  const worker_id_t& worker_id)
          : job_name_(job_name),
            job_id_ptr_(job_id_ptr),
            read_set_ptr_(read_set_ptr),
            write_set_ptr_(write_set_ptr),
            before_set_ptr_(before_set_ptr),
            after_set_ptr_(after_set_ptr),
            future_job_id_ptr_(future_job_id_ptr),
            sterile_(sterile),
            region_(region),
            worker_id_(worker_id) {type_ = COMPUTE;}

        ~ComputeJobCommandTemplate() {}

        std::string job_name_;
        JobIdPtr job_id_ptr_;
        PhyIdPtrSet read_set_ptr_;
        PhyIdPtrSet write_set_ptr_;
        JobIdPtrSet before_set_ptr_;
        JobIdPtrSet after_set_ptr_;
        JobIdPtr future_job_id_ptr_;
        bool sterile_;
        GeometricRegion region_;
        worker_id_t worker_id_;
        size_t param_index_;
    };

    class LocalCopyCommandTemplate : public CommandTemplate {
      public:
        LocalCopyCommandTemplate(JobIdPtr job_id_ptr,
                                 PhyIdPtr from_physical_data_id_ptr,
                                 PhyIdPtr to_physical_data_id_ptr,
                                 const JobIdPtrSet& before_set_ptr,
                                 const worker_id_t& worker_id)
          : job_id_ptr_(job_id_ptr),
            from_physical_data_id_ptr_(from_physical_data_id_ptr),
            to_physical_data_id_ptr_(to_physical_data_id_ptr),
            before_set_ptr_(before_set_ptr),
            worker_id_(worker_id) {type_ = LC;}

        ~LocalCopyCommandTemplate() {}

        JobIdPtr job_id_ptr_;
        PhyIdPtr from_physical_data_id_ptr_;
        PhyIdPtr to_physical_data_id_ptr_;
        JobIdPtrSet before_set_ptr_;
        worker_id_t worker_id_;
    };


    class RemoteCopySendCommandTemplate : public CommandTemplate {
      public:
        RemoteCopySendCommandTemplate(JobIdPtr job_id_ptr,
                                      JobIdPtr receive_job_id_ptr,
                                      PhyIdPtr from_physical_data_id_ptr,
                                      const ID<worker_id_t>& to_worker_id,
                                      const std::string to_ip,
                                      const ID<port_t>& to_port,
                                      const JobIdPtrSet& before_set_ptr,
                                      const worker_id_t& worker_id)
          : job_id_ptr_(job_id_ptr),
            receive_job_id_ptr_(receive_job_id_ptr),
            from_physical_data_id_ptr_(from_physical_data_id_ptr),
            to_worker_id_(to_worker_id),
            to_ip_(to_ip),
            to_port_(to_port),
            before_set_ptr_(before_set_ptr),
            worker_id_(worker_id) {type_ = RCS;}

        ~RemoteCopySendCommandTemplate() {}

        JobIdPtr job_id_ptr_;
        JobIdPtr receive_job_id_ptr_;
        PhyIdPtr from_physical_data_id_ptr_;
        ID<worker_id_t> to_worker_id_;
        std::string to_ip_;
        ID<port_t> to_port_;
        JobIdPtrSet before_set_ptr_;
        worker_id_t worker_id_;
    };

    class RemoteCopyReceiveCommandTemplate : public CommandTemplate {
      public:
        RemoteCopyReceiveCommandTemplate(JobIdPtr job_id_ptr,
                                         PhyIdPtr to_physical_data_id_ptr,
                                         const JobIdPtrSet& before_set_ptr,
                                         const worker_id_t& worker_id)
          : job_id_ptr_(job_id_ptr),
            to_physical_data_id_ptr_(to_physical_data_id_ptr),
            before_set_ptr_(before_set_ptr),
            worker_id_(worker_id) {type_ = RCR;}

        ~RemoteCopyReceiveCommandTemplate() {}

        JobIdPtr job_id_ptr_;
        PhyIdPtr to_physical_data_id_ptr_;
        JobIdPtrSet before_set_ptr_;
        worker_id_t worker_id_;
    };


    bool finalized_;
    bool established_command_template_;
    TemplateEntry *template_entry_;
    std::string record_name_;
    bool worker_template_active_;
    // Currently we do not support future job - omidm
    JobIdPtr future_job_id_ptr_;

    PatternSorted ordered_entry_patterns_;

    PatternMetaData patterns_meta_data_;

    PatternMap entry_pattern_map_;
    PatternList entry_pattern_list_;

    PatternMap end_pattern_map_;
    PatternList end_pattern_list_;

    PhyIdPtrMap phy_id_map_;
    PhyIdPtrList phy_id_list_;

    JobIdPtrMap copy_job_id_map_;
    JobIdPtrList copy_job_id_list_;

    JobIdPtrMap compute_job_id_map_;
    JobIdPtrList compute_job_id_list_;

    CommandTemplateVector command_templates_;

    std::map<job_id_t, ComputeJobCommandTemplate*> job_to_command_map_;


    typedef boost::unordered_map<job_id_t, std::set<worker_id_t> > JobWorkerMap;
    typedef boost::unordered_map<physical_data_id_t, std::set<worker_id_t> > PhyWorkerMap;
    JobWorkerMap job_worker_map_;
    PhyWorkerMap phy_worker_map_;
    std::set<worker_id_t> worker_ids_;

    std::map<worker_id_t, JobIdPtrList> worker_job_ids_;
    std::map<worker_id_t, PhyIdPtrList> worker_phy_ids_;
    std::map<worker_id_t, std::vector<size_t> > worker_parameter_indices_;

    mutable boost::mutex mutex_;

    JobIdPtr GetCopyJobIdPtr(job_id_t job_id);
    bool GetCopyJobIdPtrIfExisted(job_id_t job_id, JobIdPtr *job_id_ptr);

    JobIdPtr GetExistingComputeJobIdPtr(job_id_t job_id);
    bool GetComputeJobIdPtrIfExisted(job_id_t job_id, JobIdPtr *job_id_ptr);

    PhyIdPtr GetExistingPhyIdPtr(physical_data_id_t pdid);


    void SendCommandTemplateHeaderToWorkers(SchedulerServer *server);

    void SendCommandTemplateFinalizeToWorkers(SchedulerServer *server);

    void SpawnCommandTemplateAtWorkers(const std::vector<Parameter>& parameters,
                                       SchedulerServer *server);

    void SendComputeJobCommand(ComputeJobCommandTemplate* command,
                               const Parameter& parameter,
                               SchedulerServer *server);

    void SendLocalCopyCommand(LocalCopyCommandTemplate* command,
                              SchedulerServer *server);

    void SendRemoteCopySendCommand(RemoteCopySendCommandTemplate* command,
                                   SchedulerServer *server);

    void SendRemoteCopyReceiveCommand(RemoteCopyReceiveCommandTemplate* command,
                                      SchedulerServer *server);
};
}  // namespace nimbus
#endif  // NIMBUS_SCHEDULER_BINDING_TEMPLATE_H_