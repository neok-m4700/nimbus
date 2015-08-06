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
 * Distributed Logistic Regression application
 *
 * Author: Omid Mashayekhi<omidm@stanford.edu>
 */

#include "./app.h"
#include "./job.h"
#include "./data.h"

LogisticRegression::LogisticRegression(const size_t& dimension,
                                       const size_t& iteration_num,
                                       const size_t& partition_num,
                                       const size_t& data_size_mb)
  : dimension_(dimension),
    iteration_num_(iteration_num),
    partition_num_(partition_num),
    data_size_mb_(data_size_mb) {
      // TODO(omidm): overflow?!!!
      assert(sizeof(size_t) == 8); // NOLINT
      size_t data_size_mb_per_partition = data_size_mb / partition_num;
      size_t sample_size_b = (dimension + 1) * sizeof(float); // NOLINT
      sample_num_per_partition_ = (data_size_mb_per_partition * 1e6) / sample_size_b;
      // sample_num_per_partition_ = 10;
      std::cout << "****OMID: " << sample_num_per_partition_ << std::endl;
}

LogisticRegression::~LogisticRegression() {
};


size_t LogisticRegression::iteration_num() {
  return iteration_num_;
}

size_t LogisticRegression::dimension() {
  return dimension_;
}

size_t LogisticRegression::partition_num() {
  return partition_num_;
}

size_t LogisticRegression::data_size_mb() {
  return data_size_mb_;
}

size_t LogisticRegression::sample_num_per_partition() {
  return sample_num_per_partition_;
}

void LogisticRegression::Load() {
  RegisterJob(NIMBUS_MAIN_JOB_NAME, new Main(this));
  RegisterJob(LOOP_JOB_NAME, new ForLoop(this));
  RegisterJob(GRADIENT_JOB_NAME, new Gradient(this));
  RegisterJob(REDUCE_JOB_NAME, new Reduce(this));

  RegisterData(WEIGHT_DATA_NAME, new Weight(dimension_));
  RegisterData(SAMPLE_BATCH_DATA_NAME, new SampleBatch(dimension_, sample_num_per_partition_));
};


