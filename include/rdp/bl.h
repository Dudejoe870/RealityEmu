#pragma once

#include "rdp/rdp.h"

typedef enum
{
    BL_CLR_IN,
    BL_CLR_MEM,
    BL_CLR_BL,
    BL_CLR_FOG,
    BL_1MA,
    BL_A_MEM,
    BL_A_IN,
    BL_A_FOG,
    BL_A_SHADE,
    BL_1,
    BL_0
} blinput_t;

extern char* blinput_names[];

typedef struct
{
    rgbacolor_t* shade_color;
    rgbacolor_t* mem_color;
    rgbacolor_t* combined;
} blcolorin_t;

rgbacolor_t get_bl_color(blcolorin_t colors, uint8_t cycle);