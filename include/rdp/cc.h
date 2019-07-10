#pragma once

#include "rdp/rdp.h"

typedef enum
{
    CC_COMBINED,
    CC_TEXEL_0,
    CC_TEXEL_1, // 2 Cycle mode only
    CC_PRIM_COLOR,
    CC_SHADE_COLOR,
    CC_ENV_COLOR,
    CC_KEY_CENTER,
    CC_KEY_SCALE,
    CC_COMBINED_ALPHA,
    CC_TEX_0_ALPHA,
    CC_TEX_1_ALPHA, // 2 Cycle mode only
    CC_PRIM_ALPHA,
    CC_SHADED_ALPHA,
    CC_ENV_ALPHA,
    CC_LOD_FRAC,
    CC_PRIM_LOD_FRAC,
    CC_NOISE,
    CC_CONVERT_K4,
    CC_CONVERT_K5,
    CC_ONE,
    CC_ZERO,
    CC_ONE_ALPHA,
    CC_ZERO_ALPHA,
} ccinput_t;

extern char* ccinput_names[];

typedef struct
{
    rgbacolor_t* shade_color;
    rgbacolor_t* combined; 
} cccolorin_t;

typedef struct
{
    rgbacolor_t rgba_A;
    rgbacolor_t rgba_B;
    rgbacolor_t rgba_C;
    rgbacolor_t rgba_D;

    rgbacolor_t combined;
} cccycle_t;

rgbacolor_t get_cc_color(cccolorin_t colors, uint8_t cycle);