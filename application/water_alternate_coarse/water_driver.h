//#####################################################################
// Copyright 2009, Michael Lentine.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#ifndef __WATER_DRIVER__
#define __WATER_DRIVER__

#include "data/physbam/translator_physbam.h"
#include <PhysBAM_Tools/Grids_Uniform/UNIFORM_GRID_ITERATOR_NODE.h>
#include <PhysBAM_Tools/Grids_Uniform_Advection/ADVECTION_POLICY_UNIFORM.h>
#include <PhysBAM_Tools/Grids_Uniform_Arrays/GRID_ARRAYS_POLICY_UNIFORM.h>
#include <PhysBAM_Tools/Vectors/VECTOR.h>
#include <PhysBAM_Geometry/Grids_Uniform_Level_Sets/LEVELSET_POLICY_UNIFORM.h>
#include "shared/nimbus.h"

namespace PhysBAM{


template<class TV> class WATER_EXAMPLE;

template<class TV>
class WATER_DRIVER
{
    typedef typename TV::SCALAR T;
    typedef typename TV::template REBIND<int>::TYPE TV_INT;
    typedef typename LEVELSET_POLICY<GRID<TV> >::LEVELSET T_LEVELSET;
    typedef typename ADVECTION_POLICY<GRID<TV> >::ADVECTION_SEMI_LAGRANGIAN_SCALAR T_ADVECTION_SEMI_LAGRANGIAN_SCALAR;
    typedef typename GRID_ARRAYS_POLICY<GRID<TV> >::ARRAYS_SCALAR T_ARRAYS_SCALAR;
    typedef typename GRID_ARRAYS_POLICY<GRID<TV> >::FACE_ARRAYS T_FACE_ARRAYS_SCALAR;
    typedef typename T_ARRAYS_SCALAR::template REBIND<bool>::TYPE T_ARRAYS_BOOL;
    typedef typename T_FACE_ARRAYS_SCALAR::template REBIND<bool>::TYPE T_FACE_ARRAYS_BOOL;

    TranslatorPhysBAM<TV> translator;

protected:
    WATER_EXAMPLE<TV>& example;

public:
    bool init_phase;
    int current_frame;
    T time;
    int output_number;

    WATER_DRIVER(WATER_EXAMPLE<TV>& example);
    virtual ~WATER_DRIVER();
    
    void Initialize_Nimbus_Data(const Job *job, const DataArray &da);
    void Initialize(const Job *job, const DataArray &da);
    void Advance_To_Target_Time(const T target_time);

    void Write_Output_Files(const int frame);

    void Write_Substep(const std::string& title,const int substep,const int level=0);
    void Run(RANGE<TV_INT>& domain,const T dt,const T time);

//#####################################################################
};
}
#endif
