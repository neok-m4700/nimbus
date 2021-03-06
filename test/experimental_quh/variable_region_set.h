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
  * Stores a list of variable name/geometric region pairs, and offers
  * basic operations for set union, set differentiating, and
  * intersection query.
  *
  * Maybe merge with logical data objects in the future, but should not
  * contain data id information.
  *
  * Author: Hang Qu <quhang@stanford.edu>
  */

#ifndef VARIABLE_REGION_SET_H_  // NOLINT
#define VARIABLE_REGION_SET_H_  // NOLINT

#include <list>
#include <string>
#include <utility>
#include <vector>
#include "shared/geometric_region.h"
#include "shared/logical_data_object.h"
#include "shared/nimbus_types.h"

namespace nimbus {

class VariableRegionSet {
 public:
  VariableRegionSet();
  VariableRegionSet(const std::string& variable, const GeometricRegion& region);
  // VariableRegionSet(const LogicalDataObject& ldo);
  // VariableRegionSet(CLdoVector* ldo_list);
  VariableRegionSet(const VariableRegionSet& vrs);
  virtual ~VariableRegionSet() {}
  VariableRegionSet& CopyFrom(const VariableRegionSet& vrs);
  virtual VariableRegionSet& operator+=(const VariableRegionSet right_vrs);
  // virtual VariableRegionSet& Append(const VariableRegionSet& vrs);

  // Prints out the data structures in plain text for debugging.
  virtual void DebugPrint();
  virtual bool IntersectsAndDelete(const VariableRegionSet& vrs);
  virtual bool IntersectsTest(const VariableRegionSet& vrs);

 private:
  // void operator=(VariableRegionSet) {}  // = delete
  typedef std::pair<std::string, GeometricRegion> VarRegion;
  typedef std::list<VarRegion> VarRegionList;
  VarRegionList _var_region_list;
  void Init();
  void AddRegionHelper(
      const std::string& variable,
      const GeometricRegion& origin,
      const GeometricRegion& target);
  void PushSegXHelper(
      const GeometricRegion& gr,
      std::vector<int_dimension_t>* vec);
  void PushSegYHelper(
      const GeometricRegion& gr,
      std::vector<int_dimension_t>* vec);
  void PushSegZHelper(
      const GeometricRegion& gr,
      std::vector<int_dimension_t>* vec);
  void SortAndDeduplication(std::vector<int_dimension_t>* vec);
  bool IntersectsImpl(const VariableRegionSet& vrs, bool remove_mode);
  friend VariableRegionSet operator+(
    VariableRegionSet left_vrs,
    const VariableRegionSet& right_vrs);
};

// Returns the union of the two sets. Used for test because of performance
// reasons.
VariableRegionSet operator+(
    VariableRegionSet left_vrs,
    const VariableRegionSet& right_vrs);

}  // namespace nimbus
#endif  // VARIABLE_REGION_SET_H_  // NOLINT
