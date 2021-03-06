//#####################################################################
// Copyright 2009, Avi Robinson-Mosher.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class READ_WRITE_RIGID_BODY_STATE
//#####################################################################
#ifndef __READ_WRITE_RIGID_BODY_STATE__
#define __READ_WRITE_RIGID_BODY_STATE__

#include <PhysBAM_Tools/Read_Write/Utilities/READ_WRITE.h>
#include <PhysBAM_Solids/PhysBAM_Rigids/Rigid_Bodies/RIGID_BODY_STATE.h>
namespace PhysBAM{

template<class RW,class T_GRID>
class Read_Write<RIGID_BODY_STATE<T_GRID>,RW>
{
public:
    static void Read(std::istream& input,RIGID_BODY_STATE<T_GRID>& object)
    {char version;Read_Binary<RW>(input,version,object.time);object.template Read_Frame<RW>(input,object.frame);Read_Binary<RW>(input,object.twist.linear,object.angular_momentum,object.twist.angular);assert(version==1);}

    static void Write(std::ostream& output,const RIGID_BODY_STATE<T_GRID>& object)
    {char version=1;Write_Binary<RW>(output,version,object.time);object.template Write_Frame<RW>(output,object.frame);Write_Binary<RW>(output,object.twist.linear,object.angular_momentum,object.twist.angular);}
};
}
#endif
