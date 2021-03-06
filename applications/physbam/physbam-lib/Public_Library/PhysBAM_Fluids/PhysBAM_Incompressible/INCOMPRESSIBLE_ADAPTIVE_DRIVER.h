//#####################################################################
// Copyright 2009, Michael Lentine, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#ifndef __INCOMPRESSIBLE_ADAPTIVE_DRIVER__
#define __INCOMPRESSIBLE_ADAPTIVE_DRIVER__
#include <PhysBAM_Tools/Vectors/VECTOR.h>
namespace PhysBAM{


template<class TV> class INCOMPRESSIBLE_ADAPTIVE_EXAMPLE;

template<class TV>
class INCOMPRESSIBLE_ADAPTIVE_DRIVER
{
    typedef typename TV::SCALAR T;
    typedef typename TV::template REBIND<int>::TYPE TV_INT;

protected:
    int current_frame;
    T time;
    int output_number;

    INCOMPRESSIBLE_ADAPTIVE_EXAMPLE<TV>& example;
public:

    INCOMPRESSIBLE_ADAPTIVE_DRIVER(INCOMPRESSIBLE_ADAPTIVE_EXAMPLE<TV>& example);
    virtual ~INCOMPRESSIBLE_ADAPTIVE_DRIVER();
    
    void Execute_Main_Program();
    void Initialize();
    void Advance_To_Target_Time(const T target_time);
    void Simulate_To_Frame(const int frame_input);
    void Write_Output_Files(const int frame);
    void Write_Substep(const std::string& title,const int substep,const int level=0);

//#####################################################################
};
}
#endif
