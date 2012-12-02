/**
 * @brief     target agnostic render command processing implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#define taa_UIRENDER_C_

#ifdef taa_GL_NULL
#elif defined(taa_GL_21) || defined(taa_GL_ES2)
#include "uirender_gles2.c"
#else
#include "uirender_gl11.c"
#endif

#undef taa_UIRENDER_C_
