#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    FRMT_RGBA       = 0,
    FRMT_YUV        = 1,
    FRMT_COLORINDEX = 2,
    FRMT_IA         = 3,
    FRMT_I          = 4,
} imageformat_t;

typedef enum
{
    BPP_4  = 0,
    BPP_8  = 1,
    BPP_16 = 2,
    BPP_32 = 3
} bpp_t;

typedef struct
{
    uint32_t      image_addr;
    imageformat_t image_format;
    bpp_t         image_size;
    uint16_t      image_width;
} image_t;

image_t  curr_colorimage;
image_t  curr_teximage;
uint32_t curr_zimage_addr;

typedef struct
{
    uint16_t XH, YH, XL, YL;
} rect_t;

typedef struct
{
    rect_t  rect;
    uint8_t tile;

    uint16_t S, T;
    uint16_t DsDx, DtDy;
} texrect_t;

typedef struct
{  
    rect_t border;
    bool f, o;
} scissorborder_t;

scissorborder_t scissor_border;

typedef struct
{
    bool lft;
    uint8_t level;
    uint8_t tile;
    
    uint16_t YL, YM, YH;

    uint16_t XL, XL_frac;
    uint16_t DxLDy, DxLDy_frac;

    uint16_t XH, XH_frac;
    uint16_t DxHDy, DxHDy_frac;

    uint16_t XM, XM_frac;
    uint16_t DxMDy, DxMDy_frac;
} edgecoeff_t;

typedef struct
{
    uint16_t red, green, blue, alpha;

    uint16_t DrDx, DgDx, DbDx, DaDx;
    
    uint16_t red_frac, green_frac, blue_frac, alpha_frac;

    uint16_t DrDx_frac, DgDx_frac, DbDx_frac, DaDx_frac;

    uint16_t DrDe, DgDe, DbDe, DaDe;
    
    uint16_t DrDy, DgDy, DbDy, DaDy;
    
    uint16_t DrDe_frac, DgDe_frac, DbDe_frac, DaDe_frac;
    
    uint16_t DrDy_frac, DgDy_frac, DbDy_frac, DaDy_frac;
} shadecoeff_t;

typedef struct
{
    uint16_t S, T, W;

    uint16_t DsDx, DtDx, DwDx;

    uint16_t S_frac, T_frac, W_frac;

    uint16_t DsDx_frac, DtDx_frac, DwDx_frac;

    uint16_t DsDe, DtDe, DwDe;

    uint16_t DsDy, DtDy, DwDy;

    uint16_t DsDe_frac, DtDe_frac, DwDe_frac;

    uint16_t DsDy_frac, DtDy_frac, DwDy_frac;
} texcoeff_t;

typedef struct
{
    uint16_t Z, Z_frac;
    uint16_t DzDx, DzDx_frac;

    uint16_t DzDe, DzDe_frac;
    uint16_t DzDy, DzDy_frac;
} zbuffercoeff_t;

typedef struct
{
    uint8_t sub_a_R_0;
    uint8_t mul_R_0;
    uint8_t sub_a_A_0;
    uint8_t mul_A_0;
    uint8_t sub_a_R_1;
    uint8_t mul_R_1;
    uint8_t sub_b_R_0;
    uint8_t sub_b_R_1;
    uint8_t sub_a_A_1;
    uint8_t mul_A_1;
    uint8_t add_R_0;
    uint8_t sub_b_A_0;
    uint8_t add_A_0;
    uint8_t add_R_1;
    uint8_t sub_b_A_1;
    uint8_t add_A_1;
} combinemode_t;

combinemode_t curr_combine_mode;

typedef enum
{
    CYCLE_1    = 0,
    CYCLE_2    = 1,
    CYCLE_COPY = 2,
    CYCLE_FILL = 3
} cycletype_t;

typedef enum
{
    TLUT_RGBA5551 = 0,
    TLUT_IA88     = 1
} tluttype_t;

typedef enum
{
    SAMPLE_1X1 = 0,
    SAMPLE_2X2 = 1
} sampletype_t;

typedef enum
{
    RGBDITHER_MAGICSQUAREMATRIX = 0,
    RGBDITHER_BAYERMATRIX       = 1,
    RGBDITHER_NOISE             = 2,
    RGBDITHER_NONE              = 3
} rgbdither_t;

typedef enum
{
    ALPHADITHER_PATTERN  = 0,
    ALPHADITHER_NOTPATTERN = 1,
    ALPHADITHER_NOISE    = 2,
    ALPHADITHER_NONE     = 3
} alphadither_t;

typedef enum
{
    ZMODE_OPAQUE           = 0,
    ZMODE_INTERPENETRATING = 1,
    ZMODE_TRANSPARENT      = 2,
    ZMODE_DECAL            = 3
} zmode_t;

typedef enum
{
    CLAMP = 0,
    WRAP  = 1,
    ZAP   = 2,
    SAVE  = 3
} cvgdest_t;

typedef struct
{
    bool atomic_prim;

    cycletype_t cycle_type;

    bool persp_tex_en;
    bool detail_tex_en;
    bool sharpen_tex_en;
    bool tex_lod_en;
    bool en_tlut;

    tluttype_t   tlut_type;
    sampletype_t sample_type;
    
    bool mid_texel;
    bool bi_lerp_0;
    bool bi_lerp_1;
    bool convert_one;
    bool key_en;

    rgbdither_t   rgb_dither_sel;
    alphadither_t alpha_dither_sel;

    uint8_t B_M1A_0;
    uint8_t B_M1A_1;
    uint8_t B_M1B_0;
    uint8_t B_M1B_1;

    uint8_t B_M2A_0;
    uint8_t B_M2A_1;
    uint8_t B_M2B_0;
    uint8_t B_M2B_1;

    bool force_blend;
    bool alpha_cvg_select;
    bool cvg_times_alpha;

    zmode_t   z_mode;
    cvgdest_t cvg_dest;

    bool color_on_cvg;
    bool image_read_en;
    bool z_update_en;
    bool z_compare_en;
    bool anti_alias_en;
    bool z_source_sel;
    bool dither_alpha_en;
    bool alpha_compare_en;
} othermodes_t;

othermodes_t othermodes;

typedef struct
{
    uint8_t red, green, blue, alpha;
} rgbacolor_t;

rgbacolor_t curr_envcolor;
rgbacolor_t curr_blendcolor;
rgbacolor_t curr_fogcolor;

typedef struct
{
    uint8_t prim_min_level, prim_level_frac;
    rgbacolor_t color;
} primcolor_t;

primcolor_t curr_primcolor;

uint32_t fill_color;

typedef struct
{
    uint16_t prim_z, prim_delta_z;
} primdepth_t;

primdepth_t curr_primdepth;

typedef struct
{
    uint16_t K0, K1, K2, K3, K4, K5;
} convert_t;

convert_t curr_convert;

typedef struct
{
    uint16_t width_r;
    uint8_t  center_r;
    uint8_t  scale_r;
} keyr_t;

keyr_t curr_keyr;

typedef struct
{
    uint16_t width_g, width_b;

    uint8_t center_g, scale_g;
    uint8_t center_b, scale_b;
} keygb_t;

keygb_t curr_keygb;

bool should_run;

void RDP_init  (void);
void RDP_wake_up(void);