/**
 * @brief     target agnostic render command processing implementation
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#include <taaui/uirender.h>

#define TAA_UIRENDER_C_

#if taa_RENDER_GL11
#include "uirender_gl11.c"
#elif taa_RENDER_NULL
#else
#error render mode not supported
#endif

#undef TAA_UIRENDER_C_
