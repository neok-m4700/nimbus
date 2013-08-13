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
  * A Nimbus scheduler's abstraction of a worker.
  *
  * Author: Philip Levis <pal@cs.stanford.edu>
  */

#include "lib/scheduler_worker.h"
#include "lib/dbg.h"

namespace nimbus {

#define WORKER_BUFSIZE 10240

SchedulerWorker::SchedulerWorker(worker_id_t id,
                                   SchedulerServerConnection* conn,
                                   Application* app) {
  worker_id_ = id;
  connection_ = conn;
  application_ = app;
  is_alive_ = true;
  read_buffer_ = new uint8_t[WORKER_BUFSIZE];
}

SchedulerWorker::~SchedulerWorker() {
  // delete connection_;
}

worker_id_t SchedulerWorker::worker_id() {
  return worker_id_;
}

SchedulerServerConnection* SchedulerWorker::connection() {
  return connection_;
}

Application* SchedulerWorker::application() {
  return application_;
}

bool SchedulerWorker::is_alive() {
  return is_alive_;
}

void SchedulerWorker::MarkDead() {
  is_alive_ = false;
}

uint8_t* SchedulerWorker::read_buffer() {
  return read_buffer_;
}

uint32_t SchedulerWorker::read_buffer_length() {
  return WORKER_BUFSIZE;
}

}  // namespace nimbus
