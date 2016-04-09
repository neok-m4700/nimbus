//#####################################################################
// Copyright 2005-2006, Geoffrey Irving, Frank Losasso, Jonathan Su.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PCG_SPARSE_MPI
//#####################################################################
#ifndef __NIMBUS_PCG_SPARSE_MPI__
#define __NIMBUS_PCG_SPARSE_MPI__

#ifdef USE_MPI

#include <PhysBAM_Tools/Grids_Uniform/GRID.h>
#include <PhysBAM_Tools/Krylov_Solvers/PCG_SPARSE.h>
#include <PhysBAM_Tools/Parallel_Computation/MPI_UTILITIES.h>
#include <PhysBAM_Tools/Parallel_Computation/SPARSE_MATRIX_PARTITION.h>
#include <PhysBAM_Tools/Parallel_Computation/MPI_UNIFORM_GRID.h>
#include "projection_data.h"

namespace PhysBAM{

class SPARSE_MATRIX_PARTITION;

template<class T_GRID>
class NIMBUS_PCG_SPARSE_MPI:public NONCOPYABLE
{
public:
    typedef typename T_GRID::VECTOR_T TV;typedef typename TV::SCALAR T;typedef typename T_GRID::VECTOR_INT TV_INT;
    PCG_SPARSE<T>& pcg;
    MPI::Intracomm& comm;
    THREADED_UNIFORM_GRID<T_GRID>* thread_grid;
    MPI_THREADED_UNIFORM_GRID<T_GRID>* mpi_threaded_grid;
    SPARSE_MATRIX_PARTITION& partition;
    ARRAY<MPI::Datatype> boundary_datatypes,ghost_datatypes;
    ARRAY<ARRAY<int> > columns_to_send;
    ARRAY<ARRAY<int> > columns_to_receive;

    NIMBUS_PCG_SPARSE_MPI(PCG_SPARSE<T>& pcg_input,MPI::Intracomm& comm_input,SPARSE_MATRIX_PARTITION& partition_input)
        :pcg(pcg_input),comm(comm_input),thread_grid(0),mpi_threaded_grid(0),partition(partition_input)
    {}

    virtual ~NIMBUS_PCG_SPARSE_MPI()
    {MPI_UTILITIES::Free_Elements_And_Clean_Memory(boundary_datatypes);MPI_UTILITIES::Free_Elements_And_Clean_Memory(ghost_datatypes);}

    // TODO
    template<class TYPE> TYPE Global_Sum(const TYPE& input)
    {
      TYPE output;
      MPI_UTILITIES::Reduce(input,output,MPI::SUM,comm);
      return output;
    }

    // TODO
    template<class TYPE> TYPE Global_Max(const TYPE& input)
    {
       TYPE output;MPI_UTILITIES::Reduce(input,output,MPI::MAX,comm);
      return output;
     }

    // TODO
    virtual void Fill_Ghost_Cells(VECTOR_ND<T>& v)
    {
      ARRAY<MPI::Request> requests;
      requests.Preallocate(2*partition.number_of_sides);
      for(int s=1;s<=partition.number_of_sides;s++)
	if(boundary_datatypes(s)!=MPI::DATATYPE_NULL)
	  requests.Append(comm.Isend(v.x-1,1,boundary_datatypes(s),partition.neighbor_ranks(s),s));
      for(int s=1;s<=partition.number_of_sides;s++)
	if(ghost_datatypes(s)!=MPI::DATATYPE_NULL)
	  requests.Append(comm.Irecv(v.x-1,1,ghost_datatypes(s),partition.neighbor_ranks(s),((s-1)^1)+1));
      MPI_UTILITIES::Wait_All(requests);
    }
    
//#####################################################################
    virtual void Initialize_Datatypes();

    void Initialize(
        ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
    void CommunicateConfig( 
        ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
    void Parallel_Solve(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);

      void ExchangePressure(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void InitializeResidual(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void SpawnFirstIteration(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void DoPrecondition(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void CalculateBeta(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void UpdateSearchVector(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void ExchangeSearchVector(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void UpdateTempVector(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void CalculateAlpha(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void UpdateOtherVectors(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void CalculateResidual(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
      void DecideToSpawnNextIteration(
    	ProjectionInternalData<TV>* projection_internal_data,
        ProjectionData<TV>* projection_data);
//#####################################################################
};
}
#endif
#endif