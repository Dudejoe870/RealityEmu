#include "rdp/cc.h"

#include "common.h"

char* ccinput_names[] =
{
    "CC_COMBINED_0",
    "CC_TEXEL_0",
    "CC_TEXEL_1",
    "CC_PRIM_COLOR",
    "CC_SHADE_COLOR",
    "CC_ENV_COLOR",
    "CC_KEY_CENTER",
    "CC_KEY_SCALE",
    "CC_COMBINED_0_ALPHA",
    "CC_TEX_0_ALPHA",
    "CC_TEX_1_ALPHA",
    "CC_PRIM_ALPHA",
    "CC_SHADED_ALPHA",
    "CC_ENV_ALPHA",
    "CC_LOD_FRAC",
    "CC_PRIM_LOD_FRAC",
    "CC_NOISE",
    "CC_CONVERT_K4",
    "CC_CONVERT_K5",
    "CC_ONE",
    "CC_ZERO"
};

__attribute__((__always_inline__)) static inline ccinput_t get_cc_a(uint8_t cycle, bool alpha)
{
    uint8_t suba;
    if (alpha) 
    {
        suba = (cycle == 0) ? combinemode.sub_a_A_0 : combinemode.sub_a_A_1;

        switch (suba)
        {
            case 0:  return CC_COMBINED_ALPHA;
            case 1:  return CC_TEX_0_ALPHA;
            case 2:  return CC_TEX_1_ALPHA;
            case 3:  return CC_PRIM_ALPHA;
            case 4:  return CC_SHADED_ALPHA;
            case 5:  return CC_ENV_ALPHA;
            case 6:  return CC_ONE;
            default: return CC_ZERO;
        }
    }
    else       
    {
        suba = (cycle == 0) ? combinemode.sub_a_R_0 : combinemode.sub_a_R_1;

        switch (suba)
        {
            case 0:  return CC_COMBINED;
            case 1:  return CC_TEXEL_0;
            case 2:  return CC_TEXEL_1;
            case 3:  return CC_PRIM_COLOR;
            case 4:  return CC_SHADE_COLOR;
            case 5:  return CC_ENV_COLOR;
            case 6:  return CC_ONE;
            case 7:  return CC_NOISE;
            default: return CC_ZERO;
        }
    }
}

__attribute__((__always_inline__)) static inline ccinput_t get_cc_b(uint8_t cycle, bool alpha)
{
    uint8_t subb;
    if (alpha) 
    {
        subb = (cycle == 0) ? combinemode.sub_b_A_0 : combinemode.sub_b_A_1;

        switch (subb)
        {
            case 0:  return CC_COMBINED_ALPHA;
            case 1:  return CC_TEX_0_ALPHA;
            case 2:  return CC_TEX_1_ALPHA;
            case 3:  return CC_PRIM_ALPHA;
            case 4:  return CC_SHADED_ALPHA;
            case 5:  return CC_ENV_ALPHA;
            case 6:  return CC_ONE;
            default: return CC_ZERO;
        }
    }
    else
    {
        subb = (cycle == 0) ? combinemode.sub_b_R_0 : combinemode.sub_b_R_1;

        switch (subb)
        {
            case 0:  return CC_COMBINED;
            case 1:  return CC_TEXEL_0;
            case 2:  return CC_TEXEL_1;
            case 3:  return CC_PRIM_COLOR;
            case 4:  return CC_SHADE_COLOR;
            case 5:  return CC_ENV_COLOR;
            case 6:  return CC_KEY_CENTER;
            case 7:  return CC_CONVERT_K4;
            default: return CC_ZERO;
        }
    }
}

__attribute__((__always_inline__)) static inline ccinput_t get_cc_c(uint8_t cycle, bool alpha)
{
    uint8_t mulc;
    if (alpha) 
    {
        mulc = (cycle == 0) ? combinemode.mul_A_0 : combinemode.mul_A_1;

        switch (mulc)
        {
            case 0:  return CC_LOD_FRAC;
            case 1:  return CC_TEX_0_ALPHA;
            case 2:  return CC_TEX_1_ALPHA;
            case 3:  return CC_PRIM_ALPHA;
            case 4:  return CC_SHADED_ALPHA;
            case 5:  return CC_ENV_ALPHA;
            case 6:  return CC_PRIM_LOD_FRAC;
            default: return CC_ZERO;
        }
    }
    else       
    {
        mulc = (cycle == 0) ? combinemode.mul_R_0 : combinemode.mul_R_1;

        switch (mulc)
        {
            case 0:  return CC_COMBINED;
            case 1:  return CC_TEXEL_0;
            case 2:  return CC_TEXEL_1;
            case 3:  return CC_PRIM_COLOR;
            case 4:  return CC_SHADE_COLOR;
            case 5:  return CC_ENV_COLOR;
            case 6:  return CC_KEY_SCALE;
            case 7:  return CC_COMBINED_ALPHA;
            case 8:  return CC_TEX_0_ALPHA;
            case 9:  return CC_TEX_1_ALPHA;
            case 10: return CC_PRIM_ALPHA;
            case 11: return CC_SHADED_ALPHA;
            case 12: return CC_ENV_ALPHA;
            case 13: return CC_LOD_FRAC;
            case 14: return CC_PRIM_LOD_FRAC;
            case 15: return CC_CONVERT_K5;
            default: return CC_ZERO;
        }
    }
}

__attribute__((__always_inline__)) static inline ccinput_t get_cc_d(uint8_t cycle, bool alpha)
{
    uint8_t addd;
    if (alpha) 
    {
        addd = (cycle == 0) ? combinemode.add_A_0 : combinemode.add_A_1;

        switch (addd)
        {
            case 0:  return CC_COMBINED_ALPHA;
            case 1:  return CC_TEX_0_ALPHA;
            case 2:  return CC_TEX_1_ALPHA;
            case 3:  return CC_PRIM_ALPHA;
            case 4:  return CC_SHADED_ALPHA;
            case 5:  return CC_ENV_ALPHA;
            case 6:  return CC_ONE;
            default: return CC_ZERO;
        }
    }
    else       
    {
        addd = (cycle == 0) ? combinemode.add_R_0 : combinemode.add_R_1;

        switch (addd)
        {
            case 0:  return CC_COMBINED;
            case 1:  return CC_TEXEL_0;
            case 2:  return CC_TEXEL_1;
            case 3:  return CC_PRIM_COLOR;
            case 4:  return CC_SHADE_COLOR;
            case 5:  return CC_ENV_COLOR;
            case 6:  return CC_ONE;
            default: return CC_ZERO;
        }
    }
}

__attribute__((__always_inline__)) static inline void unimplemented_ccinput(ccinput_t val)
{
    printf("WARNING: Unimplemented CC Input %s!\n", ccinput_names[val]);
}

void get_ccinput_val_a(ccinput_t in, cccolorin_t colors, rgbacolor_t* out)
{
    switch (in)
    {
        case CC_LOD_FRAC:
            out->alpha = 255;
            return;
        case CC_COMBINED_ALPHA:
            if (colors.combined)
                out->alpha = colors.combined->alpha;
            return;
        case CC_TEX_0_ALPHA:
            if (colors.texel0_color)
                out->alpha = colors.texel0_color->alpha;
            return;
        case CC_TEX_1_ALPHA:
            if (colors.texel1_color)
                out->alpha = colors.texel1_color->alpha;
            return;
        case CC_PRIM_ALPHA:
            out->alpha = curr_primcolor.color.alpha;
            return;
        case CC_SHADED_ALPHA:
            if (colors.shade_color)
                out->alpha = colors.shade_color->alpha;
            return;
        case CC_ENV_ALPHA:
            out->alpha = curr_envcolor.alpha;
            return;
        case CC_ONE:
            out->alpha = 255;
            return;
        case CC_ZERO:
            out->alpha = 0;
            return;
        default:
            if (config.cc_logging) unimplemented_ccinput(in);
            return;
    }
}

void get_ccinput_val_r(ccinput_t in, cccolorin_t colors, rgbacolor_t* out)
{
    switch (in)
    {
        case CC_COMBINED:
            if (colors.combined)
                memcpy(out, colors.combined, sizeof(rgbacolor_t));
            return;
        case CC_TEXEL_0:
            if (colors.texel0_color)
                memcpy(out, colors.texel0_color, sizeof(rgbacolor_t));
            return;
        case CC_TEXEL_1:
            if (colors.texel1_color)
                memcpy(out, colors.texel1_color, sizeof(rgbacolor_t));
            return;
        case CC_PRIM_COLOR:
            memcpy(out, &curr_primcolor.color, sizeof(rgbacolor_t));
            return;
        case CC_SHADE_COLOR:
            if (colors.shade_color)
                memcpy(out, colors.shade_color, sizeof(rgbacolor_t));
            return;
        case CC_ENV_COLOR:
            memcpy(out, &curr_envcolor, sizeof(rgbacolor_t));
            return;
        case CC_LOD_FRAC:
            out->red   = 255;
            out->green = 255;
            out->blue  = 255;
            out->alpha = 255;
            return;
        case CC_COMBINED_ALPHA:
            if (colors.combined)
                memset(out, colors.combined->alpha, sizeof(rgbacolor_t));
            return;
        case CC_TEX_0_ALPHA:
            if (colors.texel0_color)
                memset(out, colors.texel0_color->alpha, sizeof(rgbacolor_t));
            return;
        case CC_TEX_1_ALPHA:
            if (colors.texel1_color)
                memset(out, colors.texel1_color->alpha, sizeof(rgbacolor_t));
            return;
        case CC_PRIM_ALPHA:
            memset(out, curr_primcolor.color.alpha, sizeof(rgbacolor_t));
            return;
        case CC_SHADED_ALPHA:
            if (colors.shade_color)
                memset(out, colors.shade_color->alpha, sizeof(rgbacolor_t));
            return;
        case CC_ENV_ALPHA:
            memset(out, curr_envcolor.alpha, sizeof(rgbacolor_t));
            return;
        case CC_ONE:
            memset(out, 255, sizeof(rgbacolor_t));
            return;
        case CC_ZERO:
            memset(out, 0, sizeof(rgbacolor_t));
            return;
        default:
            if (config.cc_logging) unimplemented_ccinput(in);
            return;
    }
}

rgbacolor_t get_cc_color(cccolorin_t colors, uint8_t cycle)
{
    rgbacolor_t res;

    rgbacolor_t rgba_A;
    rgbacolor_t rgba_B;
    rgbacolor_t rgba_C;
    rgbacolor_t rgba_D;
    memset(&rgba_A, 0, sizeof(rgbacolor_t));
    memset(&rgba_B, 0, sizeof(rgbacolor_t));
    memset(&rgba_C, 0, sizeof(rgbacolor_t));
    memset(&rgba_D, 0, sizeof(rgbacolor_t));

    ccinput_t r_A = get_cc_a(cycle, false);
    ccinput_t r_B = get_cc_b(cycle, false);
    ccinput_t r_C = get_cc_c(cycle, false);
    ccinput_t r_D = get_cc_d(cycle, false);
    
    //printf("Color = (%s - %s) * %s + %s\n", ccinput_names[r_A], ccinput_names[r_B], ccinput_names[r_C], ccinput_names[r_D]);

    get_ccinput_val_r(r_A, colors, &rgba_A);
    get_ccinput_val_r(r_B, colors, &rgba_B);
    get_ccinput_val_r(r_C, colors, &rgba_C);
    get_ccinput_val_r(r_D, colors, &rgba_D);

    ccinput_t a_A = get_cc_a(cycle, true);
    ccinput_t a_B = get_cc_b(cycle, true);
    ccinput_t a_C = get_cc_c(cycle, true);
    ccinput_t a_D = get_cc_d(cycle, true);

    //printf("Alpha = (%s - %s) * %s + %s\n", ccinput_names[a_A], ccinput_names[a_B], ccinput_names[a_C], ccinput_names[a_D]);

    get_ccinput_val_a(a_A, colors, &rgba_A);
    get_ccinput_val_a(a_B, colors, &rgba_B);
    get_ccinput_val_a(a_C, colors, &rgba_C);
    get_ccinput_val_a(a_D, colors, &rgba_D);

    res.red   = (((rgba_A.red   - rgba_B.red)   * rgba_C.red)   / 255) + rgba_D.red;
    res.green = (((rgba_A.green - rgba_B.green) * rgba_C.green) / 255) + rgba_D.green;
    res.blue  = (((rgba_A.blue  - rgba_B.blue)  * rgba_C.blue)  / 255) + rgba_D.blue;
    res.alpha = (((rgba_A.alpha - rgba_B.alpha) * rgba_C.alpha) / 255) + rgba_D.alpha;

    return res;
}