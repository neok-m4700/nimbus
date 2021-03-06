
// --------------------------------------------------------
// Generated by glux perl script (Wed Mar 31 17:19:36 2004)
// 
// Sylvain Lefebvre - 2002 - Sylvain.Lefebvre@imag.fr
// --------------------------------------------------------
#include "glux_no_redefine.h"
#include "glux_ext_defs.h"
#include "gluxLoader.h"
#include "gluxPlugin.h"
// --------------------------------------------------------
//         Plugin creation
// --------------------------------------------------------

#ifndef __GLUX_GL_ATI_pn_triangles__
#define __GLUX_GL_ATI_pn_triangles__

GLUX_NEW_PLUGIN(GL_ATI_pn_triangles);
// --------------------------------------------------------
//           Extension conditions
// --------------------------------------------------------
// --------------------------------------------------------
//           Extension defines
// --------------------------------------------------------
#ifndef GL_PN_TRIANGLES_ATI
#  define GL_PN_TRIANGLES_ATI 0x87F0
#endif
#ifndef GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI
#  define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI 0x87F1
#endif
#ifndef GL_PN_TRIANGLES_POINT_MODE_ATI
#  define GL_PN_TRIANGLES_POINT_MODE_ATI 0x87F2
#endif
#ifndef GL_PN_TRIANGLES_NORMAL_MODE_ATI
#  define GL_PN_TRIANGLES_NORMAL_MODE_ATI 0x87F3
#endif
#ifndef GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI
#  define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI 0x87F4
#endif
#ifndef GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI
#  define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI 0x87F5
#endif
#ifndef GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI
#  define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI 0x87F6
#endif
#ifndef GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI
#  define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI 0x87F7
#endif
#ifndef GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI
#  define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI 0x87F8
#endif
// --------------------------------------------------------
//           Extension gl function typedefs
// --------------------------------------------------------
#ifndef __GLUX__GLFCT_glPNTrianglesiATI
typedef void (APIENTRYP PFNGLUXPNTRIANGLESIATIPROC) (GLenum pname, GLint param);
#endif
#ifndef __GLUX__GLFCT_glPNTrianglesfATI
typedef void (APIENTRYP PFNGLUXPNTRIANGLESFATIPROC) (GLenum pname, GLfloat param);
#endif
// --------------------------------------------------------
//           Extension gl functions
// --------------------------------------------------------
namespace glux {
#ifndef __GLUX__GLFCT_glPNTrianglesiATI
extern PFNGLUXPNTRIANGLESIATIPROC glPNTrianglesiATI;
#endif
#ifndef __GLUX__GLFCT_glPNTrianglesfATI
extern PFNGLUXPNTRIANGLESFATIPROC glPNTrianglesfATI;
#endif
} // namespace glux
// --------------------------------------------------------
#endif
// --------------------------------------------------------
