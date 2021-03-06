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
  * Abstraction of a connection between two Nimbus workers.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */


#ifndef NIMBUS_SRC_SHARED_WORKER_DATA_EXCHANGER_CONNECTION_H_
#define NIMBUS_SRC_SHARED_WORKER_DATA_EXCHANGER_CONNECTION_H_

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_array.hpp>
#include <map>
#include <list>
#include <string>
#include "src/shared/dbg.h"
#include "src/shared/parser.h"
#include "src/shared/nimbus_types.h"
#include "src/shared/serialized_data.h"

namespace nimbus {

using boost::asio::ip::tcp;

class WorkerDataExchangerConnection {
 public:
  explicit WorkerDataExchangerConnection(tcp::socket* sock);
  virtual ~WorkerDataExchangerConnection();

  void AllocateData(size_t size);

  void AppendData(char* buffer, size_t size);

  tcp::socket* socket();
  job_id_t receive_job_id();
  job_id_t mega_rcr_job_id();
  template_id_t template_generation_id();
  boost::shared_array<char> data_ptr();
  char* read_buffer();
  size_t existing_bytes();
  size_t read_buffer_max_length();
  size_t data_length();
  data_version_t data_version();
  size_t remaining_data_length();
  bool middle_of_data();
  bool middle_of_header();
  std::list<SerializedData>* send_queue();
  virtual boost::mutex* mutex();

  void set_receive_job_id(job_id_t job_id);
  void set_mega_rcr_job_id(job_id_t job_id);
  void set_template_generation_id(template_id_t template_generation_id);
  void set_data_length(size_t len);
  void set_data_version(data_version_t version);
  void set_existing_bytes(size_t len);
  void set_middle_of_data(bool flag);
  void set_middle_of_header(bool flag);

 private:
  tcp::socket* socket_;
  job_id_t receive_job_id_;
  job_id_t mega_rcr_job_id_;
  template_id_t template_generation_id_;
  boost::shared_array<char> data_ptr_;
  char* data_cursor_;
  char* read_buffer_;
  size_t existing_bytes_;
  size_t data_length_;
  data_version_t data_version_;
  size_t remaining_data_length_;
  bool middle_of_data_;
  bool middle_of_header_;
  std::list<SerializedData> send_queue_;
  boost::mutex* mutex_;
};

typedef std::list<WorkerDataExchangerConnection*>
WorkerDataExchangerConnectionList;

typedef std::map<worker_id_t, WorkerDataExchangerConnection*>
WorkerDataExchangerConnectionMap;

}  // namespace nimbus

#endif  // NIMBUS_SRC_SHARED_WORKER_DATA_EXCHANGER_CONNECTION_H_
