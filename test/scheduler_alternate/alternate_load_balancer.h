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
  * This is static version of the load balancer used in scheduler v2.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#ifndef NIMBUS_TEST_SCHEDULER_ALTERNATE_ALTERNATE_LOAD_BALANCER_H_
#define NIMBUS_TEST_SCHEDULER_ALTERNATE_ALTERNATE_LOAD_BALANCER_H_

#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <map>
#include <list>
#include <vector>
#include <string>
#include "shared/nimbus_types.h"
#include "scheduler/job_entry.h"
#include "scheduler/job_profile.h"
#include "scheduler/data_manager.h"
#include "scheduler/job_manager.h"
#include "scheduler/region_map.h"
#include "scheduler/straggler_map.h"
#include "scheduler/job_assigner.h"
#include "scheduler/load_balancer.h"
#include "shared/cluster.h"
#include "shared/id_maker.h"
#include "shared/geometric_region.h"
#include "shared/graph.h"

namespace nimbus {

  class AlternateLoadBalancer : public LoadBalancer {
  public:
    AlternateLoadBalancer();
    virtual ~AlternateLoadBalancer();

    virtual void Run();

    virtual size_t AssignReadyJobs();

    virtual void NotifyJobAssignment(const JobEntry *job);

    virtual void NotifyJobDone(const JobEntry *job);

    virtual void NotifyRegisteredWorker(SchedulerWorker *worker);


  private:
    typedef std::map<worker_id_t, SchedulerWorker*> WorkerMap;

    AlternateLoadBalancer(const AlternateLoadBalancer& other) {}

    Log log_;
    unsigned int seed_;
    WorkerMap worker_map_;

    bool SetWorkerToAssignJob(JobEntry *job);
  };

}  // namespace nimbus

#endif  // NIMBUS_TEST_SCHEDULER_ALTERNATE_ALTERNATE_LOAD_BALANCER_H_
