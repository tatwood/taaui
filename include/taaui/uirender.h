/**
 * @brief     render command processing header
 * @author    Thomas Atwood (tatwood.net)
 * @date      2011
 * @copyright unlicense / public domain
 ****************************************************************************/
#ifndef TAAUI_UIRENDER_H_
#define TAAUI_UIRENDER_H_

#include "uidrawlist.h"

taa_EXTERN_C void taa_uirender(
    uint32_t vieww,
    uint32_t viewh,
    const taa_uidrawlist* dl);

#endif // TAAUI_UIRENDER_H_
