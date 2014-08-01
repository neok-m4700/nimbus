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
  * This class keeps the lineage information about how logical data evolves as
  * it is written by jobs.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */


#include "scheduler/region_map.h"

namespace nimbus {

RegionMap::RegionMap() {
}

RegionMap::RegionMap(const Table& table)
  : table_(table) {
}

RegionMap::RegionMap(const RegionMap& other) {
  table_ = other.table_;
}

RegionMap::~RegionMap() {
}

RegionMap::Table RegionMap::table() const {
  return table_;
}

const RegionMap::Table* RegionMap::table_p() const {
  return &table_;
}

RegionMap::Table* RegionMap::table_p() {
  return &table_;
}

void RegionMap::set_table(const Table& table) {
  table_ = table;
}

RegionMap& RegionMap::operator= (
    const RegionMap& right) {
  table_ = right.table_;
  return *this;
}

}  // namespace nimbus