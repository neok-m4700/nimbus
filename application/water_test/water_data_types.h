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
 * Data types used by the application jobs and functions.
 *
 * Author: Chinmayee Shah <chinmayee.shah@stanford.edu>
 */

#ifndef NIMBUS_APPLICATION_WATER_TEST_WATER_DATA_TYPES_H_
#define NIMBUS_APPLICATION_WATER_TEST_WATER_DATA_TYPES_H_

/* Include relevant PhysBAM files here.
*/
#include "shared/nimbus.h"
#include "./physbam_include.h"

/* WATER_EXAMPLE is structured as follows (with the equivalent here shown in
 * brackets):
 * mac_grid (equivalent *mac_grid.data)
 * mpi_grid (mpi_grid.data)
 * thread_queue (NULL)
 * face_velocities (*face_velocities.data)
 * boundary_scalar (*sim_data.boundary_scalar)
 * boundary (sim_data.boundary)
 * phi_boundary (sim_data.boundary)
 * phi_boundary_water (*sim_data.phi_boundary_water)
 * domain_boundary (*sim_data.domain_boundary)
 * sources (*sim_data.sources)
 * particle_levelset_evolution (*sim_data.particle_levelset_evolution)
 * advection_scalar (*sim_data.advection_scalar)
 * rigid_geometry_collection (*sim_data.rigid_geometry_collection)
 * collision_bodies_affecting_fluid (*sim_data.collision_bodies_affecting_fluid)
 * projection (sim_data.projection)
 * incompressible (*sim_data.incompressible)
 * kinematic_evolution (*sim_data.kinematic_evolution)
 */

using namespace PhysBAM;
using nimbus::Data;

/* Face array for storing quantities like face velocities.
*/
template <class TV>
class FaceArray : public Data {
    private:
        int size_;
        typedef typename TV::SCALAR T;
    public:
        FaceArray(int size);
        virtual void create();
        virtual Data* clone();
        bool initialize();
        // physbam structures and methods
        ARRAY<T, FACE_INDEX<TV::dimension> > *data;
};

/* Ghost face array for storing scalar quantities.
*/
template <class TV>
class FaceArrayGhost : public Data {
    private:
        int size_;
    public:
        FaceArrayGhost(int size);
        virtual void create();
        virtual Data* clone();
        bool initialize();
        // physbam structures and methods
        typename GRID_ARRAYS_POLICY<GRID<TV> >::FACE_ARRAYS *data;
};

/* Grid class for storing the mac grid information.
*/
template <class TV>
class Grid : public Data {
    private:
        int size_;
        typedef typename TV::template REBIND<int>::TYPE TV_INT;
    public:
        Grid(int size);
        virtual void create();
        virtual Data* clone();
        bool initialize(
                const TV_INT &counts,
                const RANGE<TV> &box,
                const bool MAC_grid);
        // physbam structures and methods
        GRID<TV> *data;
};

/* MPIGrid class for storing MPI grid information.
 * TODO: Eventually eliminate this, and build a data structure
 * combining Grid and MPIGrid.
 */
template <class TV>
class MPIGrid : public Data {
    private:
        int size_;
    public:
        MPIGrid(int size);
        virtual void create();
        virtual Data* clone();
        bool initialize();
        // physbam structures and methods
        MPI_UNIFORM_GRID<GRID<TV> > *data;
};

/* Add all other data used by water simulation here.  DO NOT add scalar
 * values. Scalar values can be passed around directly as parameters.
 */
template <class TV, class T>
class NonAdvData : public Data {
    private:
        int size_;
    public:
        NonAdvData(int size);
        virtual void create();
        virtual Data* clone();
        bool initialize();

        // physbam structures and methods

        // boundary information
        BOUNDARY_UNIFORM<GRID<TV>, T>
            *boundary_scalar,
            *boundary,
            *phi_boundary;
        typename GEOMETRY_BOUNDARY_POLICY<GRID<TV> >::
            BOUNDARY_PHI_WATER *phi_boundary_water;
        VECTOR<VECTOR<bool, 2>, TV::dimension> *domain_boundary;

        // sources
        ARRAY<IMPLICIT_OBJECT<TV>*> *sources;

        // fluid data
        PARTICLE_LEVELSET_EVOLUTION_UNIFORM<GRID<TV> > *particle_levelset_evolution;
        ADVECTION_SEMI_LAGRANGIAN_UNIFORM<GRID<TV>, T> *advection_scalar;

        // rigid body, collision data
        RIGID_GEOMETRY_COLLECTION<TV> *rigid_geometry_collection;
        typename COLLISION_GEOMETRY_COLLECTION_POLICY<GRID<TV> >::
            GRID_BASED_COLLISION_GEOMETRY *collision_bodies_affecting_fluid;

        // other containers
        PROJECTION_DYNAMICS_UNIFORM<GRID<TV> > *projection;
        INCOMPRESSIBLE_UNIFORM<GRID<TV> > *incompressible;

        // driver data
        KINEMATIC_EVOLUTION<TV> *kinematic_evolution;
};

#endif  // NIMBUS_APPLICATION_WATER_TEST_WATER_DATA_TYPES_H_
