/*
 * Copyright 2014 Stanford University.
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
  * This command is sent from a worker to the scheduler, specifying
  * a computation that a worker should perform. The job lists
  * the read and write sets of the job (the data objects it reads and
  * writes) as logical IDs. The scheduler binds these to particular
  * physical objects and sends a compute_job_command to a worker.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  * Author: Philip Levis <pal@cs.stanford.edu>
  */

#include "shared/spawn_compute_job_command.h"

using namespace nimbus;  // NOLINT
using boost::tokenizer;
using boost::char_separator;

SpawnComputeJobCommand::SpawnComputeJobCommand() {
  name_ = SPAWN_COMPUTE_JOB_NAME;
  type_ = SPAWN_COMPUTE_JOB;
}

SpawnComputeJobCommand::SpawnComputeJobCommand(const std::string& job_name,
                                               const ID<job_id_t>& job_id,
                                               const IDSet<logical_data_id_t>& read,
                                               const IDSet<logical_data_id_t>& write,
                                               const IDSet<job_id_t>& before,
                                               const IDSet<job_id_t>& after,
                                               const ID<job_id_t>& parent_job_id,
                                               const ID<job_id_t>& future_job_id,
                                               const bool& sterile,
                                               const Parameter& params)
  : job_name_(job_name),
    job_id_(job_id),
    read_set_(read),
    write_set_(write),
    before_set_(before),
    after_set_(after),
    parent_job_id_(parent_job_id),
    future_job_id_(future_job_id),
    sterile_(sterile),
    params_(params) {
  name_ = SPAWN_COMPUTE_JOB_NAME;
  type_ = SPAWN_COMPUTE_JOB;
}

SpawnComputeJobCommand::~SpawnComputeJobCommand() {
}

SchedulerCommand* SpawnComputeJobCommand::Clone() {
  return new SpawnComputeJobCommand();
}


bool SpawnComputeJobCommand::Parse(const std::string& data) {
  SubmitComputeJobCommand cmd;
  bool result = cmd.ParseFromString(data);

  if (!result) {
    // Throw an error message
    return false;
  } else {
    ReadFromProtobuf(&cmd);
    return true;
  }
}

std::string SpawnComputeJobCommand::toString() {
  std::string str;
  SubmitComputeJobCommand cmd;
  WriteToProtobuf(&cmd);
  cmd.SerializeToString(&str);

    // Note asymmetry of parsing forces this copy.
  // That is, toString inserts the command name,
  // while Parse assumes the command name has been consumed.
  // Correspondingly, the protocol buffer doesn't have
  // the command name because its typing needs to follow
  // the higher-level parsers expectations of ASCII, but
  // we need to insert it in toString. Asymmetry is
  // a bad idea!
  std::string result = name_ + " ";
  result += str;
  return str;
}

std::string SpawnComputeJobCommand::toStringWTags() {
  std::string str;
  str += (name_ + " ");
  str += ("name:" + job_name_ + " ");
  str += ("id:" + job_id_.toString() + " ");
  str += ("read:" + read_set_.toString() + " ");
  str += ("write:" + write_set_.toString() + " ");
  str += ("before:" + before_set_.toString() + " ");
  str += ("after:" + after_set_.toString() + " ");
  str += ("parent-id:" + parent_job_id_.toString() + " ");
  str += ("params:" + params_.toString() + " ");
  if (sterile_) {
    str += "sterile";
  } else {
    str += "not_sterile";
  }
  return str;
}

std::string SpawnComputeJobCommand::job_name() {
  return job_name_;
}

ID<job_id_t> SpawnComputeJobCommand::job_id() {
  return job_id_;
}

ID<job_id_t> SpawnComputeJobCommand::parent_job_id() {
  return parent_job_id_;
}

ID<job_id_t> SpawnComputeJobCommand::future_job_id() {
  return future_job_id_;
}

IDSet<logical_data_id_t> SpawnComputeJobCommand::read_set() {
  return read_set_;
}

IDSet<logical_data_id_t> SpawnComputeJobCommand::write_set() {
  return write_set_;
}

IDSet<job_id_t> SpawnComputeJobCommand::after_set() {
  return after_set_;
}

IDSet<job_id_t> SpawnComputeJobCommand::before_set() {
  return before_set_;
}

Parameter SpawnComputeJobCommand::params() {
  return params_;
}

bool SpawnComputeJobCommand::sterile() {
  return sterile_;
}

bool SpawnComputeJobCommand::ReadFromProtobuf(const SubmitComputeJobCommand* cmd) {
  job_name_ = cmd->name();
  job_id_.set_elem(cmd->job_id());
  read_set_.ConvertFromRepeatedField(cmd->read_set().ids());
  write_set_.ConvertFromRepeatedField(cmd->write_set().ids());
  before_set_.ConvertFromRepeatedField(cmd->before_set().ids());
  after_set_.ConvertFromRepeatedField(cmd->after_set().ids());
  future_job_id_.set_elem(cmd->future_id());
  parent_job_id_.set_elem(cmd->parent_id());
  // Is this safe?
  SerializedData d(cmd->params());
  params_.set_ser_data(d);

  return true;
}

bool SpawnComputeJobCommand::WriteToProtobuf(SubmitComputeJobCommand* cmd) {
  cmd->set_name(job_name());
  cmd->set_job_id(job_id().elem());
  read_set().ConvertToRepeatedField(cmd->mutable_read_set()->mutable_ids());
  write_set().ConvertToRepeatedField(cmd->mutable_write_set()->mutable_ids());
  before_set().ConvertToRepeatedField(cmd->mutable_before_set()->mutable_ids());
  after_set().ConvertToRepeatedField(cmd->mutable_after_set()->mutable_ids());
  cmd->set_parent_id(parent_job_id().elem());
  cmd->set_future_id(future_job_id().elem());
  cmd->set_params(params().ser_data().toString());
  return true;
}
