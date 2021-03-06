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

#include "applications/physbam/water//physbam_tools.h"

namespace application {

template<typename TV> typename PhysBAM::RANGE<TV> RangeFromRegions(
        const nimbus::GeometricRegion& global_region,
        const nimbus::GeometricRegion& local_region) {
    TV start, end;
    start(1) = (float)(local_region.x() - 1) / (float)global_region.dx();
    start(2) = (float)(local_region.y() - 1) / (float)global_region.dy();
    start(3) = (float)(local_region.z() - 1) / (float)global_region.dz();
    end(1) =  (float)(local_region.x() + local_region.dx() - 1) / (float)global_region.dx();
    end(2) =  (float)(local_region.y() + local_region.dy() - 1) / (float)global_region.dy();
    end(3) =  (float)(local_region.z() + local_region.dz() - 1) / (float)global_region.dz();
    return (typename PhysBAM::RANGE<TV>(start, end));
}

typename PhysBAM::VECTOR<int, 3> CountFromRegion(
    const nimbus::GeometricRegion& local_region) {
  return (typename PhysBAM::VECTOR<int, 3>(local_region.dx(),
                                           local_region.dy(), 
                                           local_region.dz()));
}

typedef typename PhysBAM::VECTOR<float, 3> TVF;
template typename PhysBAM::RANGE<TVF> RangeFromRegions(
        const nimbus::GeometricRegion& global_region,
        const nimbus::GeometricRegion& local_region);

}  // namespace application
