#include "rdp/bl.h"

#include "common.h"

char* blinput_names[] =
{
    "BL_CLR_IN",
    "BL_CLR_MEM",
    "BL_CLR_BL",
    "BL_CLR_FOG",
    "BL_1MA",
    "BL_A_MEM",
    "BL_A_IN",
    "BL_A_FOG",
    "BL_A_SHADE",
    "BL_1",
    "BL_0"
};

__attribute__((__always_inline__)) static inline blinput_t get_bl_p(uint8_t cycle)
{
    uint8_t p = (cycle == 0) ? othermodes.b_m1a_0 : othermodes.b_m1a_1;
    switch (p)
    {
        case 0:  return BL_CLR_IN;
        case 1:  return BL_CLR_MEM;
        case 2:  return BL_CLR_BL;
        case 3:  return BL_CLR_FOG;
        default: return BL_0; // Shouldn't happen.
    }
}

__attribute__((__always_inline__)) static inline blinput_t get_bl_m(uint8_t cycle)
{
    uint8_t m = (cycle == 0) ? othermodes.b_m2a_0 : othermodes.b_m2a_1;
    switch (m)
    {
        case 0:  return BL_CLR_IN;
        case 1:  return BL_CLR_MEM;
        case 2:  return BL_CLR_BL;
        case 3:  return BL_CLR_FOG;
        default: return BL_0; // Shouldn't happen.
    }
}

__attribute__((__always_inline__)) static inline blinput_t get_bl_a(uint8_t cycle)
{
    uint8_t a = (cycle == 0) ? othermodes.b_m1b_0 : othermodes.b_m1b_1;
    switch (a)
    {
        case 0:  return BL_A_IN;
        case 1:  return BL_CLR_FOG;
        case 2:  return BL_A_SHADE;
        default: return BL_0;
    }
}

__attribute__((__always_inline__)) static inline blinput_t get_bl_b(uint8_t cycle)
{
    uint8_t b = (cycle == 0) ? othermodes.b_m2b_0 : othermodes.b_m2b_1;
    switch (b)
    {
        case 0:  return BL_1MA;
        case 1:  return BL_A_MEM;
        case 2:  return BL_1;
        default: return BL_0;
    }
}

void get_blinput_val(blinput_t in, blcolorin_t colors, rgbacolor_t* out, rgbacolor_t* rgba_A)
{
    switch (in)
    {
        case BL_CLR_IN:
            memcpy(out, colors.combined, sizeof(rgbacolor_t));
            return;
        case BL_CLR_MEM:
            memcpy(out, colors.mem_color, sizeof(rgbacolor_t));
            return;
        case BL_CLR_BL:
            memcpy(out, &curr_blendcolor, sizeof(rgbacolor_t));
            return;
        case BL_CLR_FOG:
            memcpy(out, &curr_fogcolor, sizeof(rgbacolor_t));
            return;
        case BL_1MA:
            if (rgba_A)
            {
                out->red   = 255 - rgba_A->red;
                out->green = 255 - rgba_A->green;
                out->blue  = 255 - rgba_A->blue;
                out->alpha = 255 - rgba_A->alpha;
            }
            return;
        case BL_A_MEM:
            out->red   = colors.mem_color->alpha;
            out->green = colors.mem_color->alpha;
            out->blue  = colors.mem_color->alpha;
            out->alpha = colors.mem_color->alpha;
            return;
        case BL_A_IN:
            out->red   = colors.combined->alpha;
            out->green = colors.combined->alpha;
            out->blue  = colors.combined->alpha;
            out->alpha = colors.combined->alpha;
            return;
        case BL_A_FOG:
            out->red   = curr_fogcolor.alpha;
            out->green = curr_fogcolor.alpha;
            out->blue  = curr_fogcolor.alpha;
            out->alpha = curr_fogcolor.alpha;
            return;
        case BL_A_SHADE:
            if (colors.shade_color)
            {
                out->red   = colors.shade_color->alpha;
                out->green = colors.shade_color->alpha;
                out->blue  = colors.shade_color->alpha;
                out->alpha = colors.shade_color->alpha;
            }
            return;
        case BL_1:
            memset(out, 255, sizeof(rgbacolor_t));
            return;
        case BL_0:
            memset(out, 0, sizeof(rgbacolor_t));
            return;
    }
}

rgbacolor_t get_bl_color(blcolorin_t colors, uint8_t cycle)
{
    rgbacolor_t res;

    rgbacolor_t rgba_P;
    rgbacolor_t rgba_A;
    rgbacolor_t rgba_M;
    rgbacolor_t rgba_B;
    memset(&rgba_P, 0, sizeof(rgbacolor_t));
    memset(&rgba_A, 0, sizeof(rgbacolor_t));
    memset(&rgba_M, 0, sizeof(rgbacolor_t));
    memset(&rgba_B, 0, sizeof(rgbacolor_t));

    blinput_t P = get_bl_p(cycle);
    blinput_t A = get_bl_a(cycle);
    blinput_t M = get_bl_m(cycle);
    blinput_t B = get_bl_b(cycle);

    get_blinput_val(P, colors, &rgba_P, NULL);
    get_blinput_val(A, colors, &rgba_A, NULL);
    get_blinput_val(M, colors, &rgba_M, NULL);
    get_blinput_val(B, colors, &rgba_B, &rgba_A);

    res.red   = ((rgba_A.red   * rgba_P.red)   / 255) + (rgba_B.red   * rgba_M.red)   / (rgba_A.red   + rgba_B.red);
    res.green = ((rgba_A.green * rgba_P.green) / 255) + (rgba_B.green * rgba_M.green) / (rgba_A.green + rgba_B.green);
    res.blue  = ((rgba_A.blue  * rgba_P.blue)  / 255) + (rgba_B.blue  * rgba_M.blue)  / (rgba_A.blue  + rgba_B.blue);
    res.alpha = ((rgba_A.alpha * rgba_P.alpha) / 255) + (rgba_B.alpha * rgba_M.alpha) / (rgba_A.alpha + rgba_B.alpha);

    return res;
}