#include "rdp/software_rasterizer.h"

#include "common.h"

#define SCANBUFFER_HEIGHT 480
#define MAX_WIDTH 640

__attribute__((__always_inline__)) static inline uint32_t get_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (curr_colorimage.size == BPP_32)
        return (r << 24) | (g << 16) | (b << 8) | a;
    else if (curr_colorimage.size == BPP_16)
    {
        float _r = ((float)r / 255) * 31;
        float _g = ((float)g / 255) * 31;
        float _b = ((float)b / 255) * 31;
        float _a = ((float)a / 255);

        uint8_t real_r = (uint8_t)_r;
        uint8_t real_g = (uint8_t)_g;
        uint8_t real_b = (uint8_t)_b;

        return (((real_r & 0b11111) << 11) | ((real_g & 0b11111) << 6) | ((real_b & 0b11111) << 1) | (_a > 0));
    }
    return 0;
}

__attribute__((__always_inline__)) static inline void convert_16_32(rgbacolor_t* out, uint16_t color)
{
    uint8_t r = (color & 0b1111100000000000) >> 11;
    uint8_t g = (color & 0b0000011111000000) >> 6;
    uint8_t b = (color & 0b0000000000111110) >> 1;
    uint8_t a = (color & 0b0000000000000001);

    float _r = (float)r / 31;
    float _g = (float)g / 31;
    float _b = (float)b / 31;
    float _a = (float)a;

    out->red   = (uint8_t)(_r * 255);
    out->green = (uint8_t)(_g * 255);
    out->blue  = (uint8_t)(_b * 255);
    out->alpha = (uint8_t)(_a * 255);
}

__attribute__((__always_inline__)) static inline void set_pixel_32(uint32_t x, uint32_t y, uint32_t packed_color)
{
    uint32_t screen_x1 = (uint32_t)(scissor_border.border.XH >> 2);
    uint32_t screen_y1 = (uint32_t)(scissor_border.border.YH >> 2);
    uint32_t screen_x2 = (uint32_t)(scissor_border.border.XL >> 2);
    uint32_t screen_y2 = (uint32_t)(scissor_border.border.YL >> 2);

    if ((x < screen_x1 || y < screen_y1) || (x > screen_x2 || y > screen_y2)) return;

    uint32_t index = x + y * (curr_colorimage.width+1);
    if (curr_colorimage.size == BPP_16) // For packed colors.
        index >>= 1;
    uint32_t* framebuffer = get_real_memory_loc(curr_colorimage.addr);
    framebuffer[index] = bswap_32(packed_color);
}

__attribute__((__always_inline__)) static inline rgbacolor_t get_pixel(uint32_t x, uint32_t y)
{
    rgbacolor_t res;
    memset(&res, 0, sizeof(rgbacolor_t));

    uint32_t screen_x1 = (uint32_t)(scissor_border.border.XH >> 2);
    uint32_t screen_y1 = (uint32_t)(scissor_border.border.YH >> 2);
    uint32_t screen_x2 = (uint32_t)(scissor_border.border.XL >> 2);
    uint32_t screen_y2 = (uint32_t)(scissor_border.border.YL >> 2);

    if ((x < screen_x1 || y < screen_y1) || (x > screen_x2 || y > screen_y2)) return res;

    uint32_t index = x + y * (curr_colorimage.width+1);
    
    if (curr_colorimage.size == BPP_32)
    {
        uint32_t* framebuffer = get_real_memory_loc(curr_colorimage.addr);
        uint32_t pixel = bswap_32(framebuffer[index]);
        res.red   = (pixel & 0xFF000000) >> 24;
        res.green = (pixel & 0x00FF0000) >> 16;
        res.blue  = (pixel & 0x0000FF00) >> 8;
        res.alpha = (pixel & 0x000000FF);
    }
    else if (curr_colorimage.size == BPP_16)
    {
        uint16_t* framebuffer = get_real_memory_loc(curr_colorimage.addr);
        convert_16_32(&res, bswap_16(framebuffer[index]));
    }

    return res;
}

__attribute__((__always_inline__)) static inline void set_pixel_16(uint32_t x, uint32_t y, uint16_t color)
{
    uint32_t screen_x1 = (uint32_t)(scissor_border.border.XH >> 2);
    uint32_t screen_y1 = (uint32_t)(scissor_border.border.YH >> 2);
    uint32_t screen_x2 = (uint32_t)(scissor_border.border.XL >> 2);
    uint32_t screen_y2 = (uint32_t)(scissor_border.border.YL >> 2);

    if ((x < screen_x1 || y < screen_y1) || (x > screen_x2 || y > screen_y2)) return;

    uint32_t index = x + y * (curr_colorimage.width+1);
    uint16_t* framebuffer = get_real_memory_loc(curr_colorimage.addr);
    framebuffer[index] = bswap_16(color);
}

__attribute__((__always_inline__)) static inline void set_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    if      (curr_colorimage.size == BPP_32) set_pixel_32(x, y, color);
    else if (curr_colorimage.size == BPP_16) set_pixel_16(x, y, (uint16_t)color);
}

__attribute__((__always_inline__)) static inline uint32_t process_pixel(cccolorin_t cc_colors, blcolorin_t bl_colors)
{
    if (othermodes.cycle_type == CYCLE_1 || othermodes.cycle_type == CYCLE_2)
    {
        rgbacolor_t cc_color_0;
        if (othermodes.cycle_type == CYCLE_2)
        {
            cc_color_0 = get_cc_color(cc_colors, 0);
            cc_colors.combined = &cc_color_0;
        }

        rgbacolor_t cc_color_1 = get_cc_color(cc_colors, 1);

        bl_colors.combined = &cc_color_1;
        rgbacolor_t bl_color_0 = get_bl_color(bl_colors, 0);

        rgbacolor_t bl_color_1;
        memcpy(&bl_color_1, &bl_color_0, sizeof(rgbacolor_t));
        if (othermodes.cycle_type == CYCLE_2)
        {
            bl_colors.combined = &bl_color_0;
            bl_color_1 = get_bl_color(bl_colors, 1);
        }

        return get_color(bl_color_1.red, bl_color_1.green, bl_color_1.blue, bl_color_1.alpha);
    }
    else if (othermodes.cycle_type == CYCLE_COPY && cc_colors.texel0_color)
        return get_color(cc_colors.texel0_color->red, cc_colors.texel0_color->green, cc_colors.texel0_color->blue, cc_colors.texel0_color->alpha);
    
    return 0;
}

__attribute__((__always_inline__)) static inline void get_tex_coords(uint32_t* S, uint32_t* T, float float_S, float float_T, uint8_t index, bool flip)
{
    uint32_t temp_S = (uint32_t)float_S;
    uint32_t temp_T = (uint32_t)float_T;
    if (tiles[index].shift_s > 10)
        temp_S <<= (~tiles[index].shift_s & 0x7) + 1;
    else
        temp_S >>= tiles[index].shift_s;

    if (tiles[index].shift_t > 10)
        temp_T <<= (~tiles[index].shift_t & 0x7) + 1;
    else 
        temp_T >>= tiles[index].shift_t;
    
    if (tiles[index].ms && (((temp_S >> tiles[index].mask_s) & 0x1) != 0)) temp_S = ~temp_S;
    if (tiles[index].mt && (((temp_T >> tiles[index].mask_t) & 0x1) != 0)) temp_T = ~temp_T;

    if (tiles[index].ms && tiles[index].mask_s != 0) temp_S &= (1 << (tiles[index].mask_s + 1)) - 1;
    if (tiles[index].mt && tiles[index].mask_t != 0) temp_T &= (1 << (tiles[index].mask_t + 1)) - 1;

    if (tiles[index].cs || tiles[index].mask_s == 0)
    {
        if (temp_S > tiles[index].sh)
            temp_S = tiles[index].sh;
    }

    if (tiles[index].ct || tiles[index].mask_t == 0)
    {
        if (temp_T > tiles[index].th)
            temp_T = tiles[index].th;
    }
    *S = flip ? temp_T : temp_S;
    *T = flip ? temp_S : temp_T;
}

__attribute__((__always_inline__)) static inline void do_x_pixel_scanbuffer(size_t y, int lft, uint32_t xmax, uint32_t xmin, 
                                                                            float* shd_red,  float* shd_green, float* shd_blue, float* shd_alpha,
                                                                            float* shd_DrDx, float* shd_DgDx,  float* shd_DbDx, float* shd_DaDx,
                                                                            float* shd_DrDe, float* shd_DgDe,  float* shd_DbDe, float* shd_DaDe,
                                                                            shadecoeff_t* shade)
{
    float shd_red_temp   = 0;
    float shd_green_temp = 0;
    float shd_blue_temp  = 0;
    float shd_alpha_temp = 0;

    bool is_cycles = othermodes.cycle_type == CYCLE_1 || othermodes.cycle_type == CYCLE_2;

    if (shade && is_cycles && xmax != 0)
    {
        shd_red_temp   = *shd_red;
        shd_green_temp = *shd_green;
        shd_blue_temp  = *shd_blue;
        shd_alpha_temp = *shd_alpha;
    }

    size_t x = (lft == 0) ? xmax : xmin;
    while (true)
    {
        if (lft == 0)
        {
            if (x <= xmin) break;
        }
        else if (lft == 1)
        {
            if (x >= xmax) break;
        }

        if (xmax == 0) break;
        uint32_t color = 0;

        if (othermodes.cycle_type == CYCLE_FILL)
            color = fill_color;
        else if (is_cycles)
        {
            rgbacolor_t shd_color;
            memset(&shd_color, 0, sizeof(shd_color));
            bool edge_cond = false;

            if (lft == 0) edge_cond = x == xmax;
            else          edge_cond = x == xmin;
            if (shade)
            {
                if (edge_cond)
                {
                    shd_red_temp   += *shd_DrDe;
                    shd_green_temp += *shd_DgDe;
                    shd_blue_temp  += *shd_DbDe;
                    shd_alpha_temp += *shd_DaDe;

                    *shd_red   += *shd_DrDe;
                    *shd_green += *shd_DgDe;
                    *shd_blue  += *shd_DbDe;
                    *shd_alpha += *shd_DaDe;

                    if (shd_red_temp   < 0) shd_red_temp   = 0;
                    if (shd_green_temp < 0) shd_green_temp = 0;
                    if (shd_blue_temp  < 0) shd_green_temp = 0;
                    if (shd_alpha_temp < 0) shd_green_temp = 0;

                    if (*shd_red   < 0) *shd_red   = 0;
                    if (*shd_green < 0) *shd_green = 0;
                    if (*shd_blue  < 0) *shd_blue  = 0;
                    if (*shd_alpha < 0) *shd_alpha = 0;
                }

                if (lft == 0)
                {
                    *shd_red   -= *shd_DrDx;
                    *shd_green -= *shd_DgDx;
                    *shd_blue  -= *shd_DbDx;
                    *shd_alpha -= *shd_DaDx;
                }
                else if (lft == 1)
                {
                    *shd_red   += *shd_DrDx;
                    *shd_green += *shd_DgDx;
                    *shd_blue  += *shd_DbDx;
                    *shd_alpha += *shd_DaDx;
                }

                if (*shd_red   < 0) *shd_red   = 0;
                if (*shd_green < 0) *shd_green = 0;
                if (*shd_blue  < 0) *shd_blue  = 0;
                if (*shd_alpha < 0) *shd_alpha = 0;

                shd_color.red   = (uint8_t)*shd_red;
                shd_color.green = (uint8_t)*shd_green;
                shd_color.blue  = (uint8_t)*shd_blue;
                shd_color.alpha = (uint8_t)*shd_alpha;
            }

            cccolorin_t cc_colors; 
            cc_colors.combined     = NULL;
            cc_colors.texel0_color = NULL;
            cc_colors.texel1_color = NULL;
            cc_colors.shade_color  = &shd_color;

            blcolorin_t bl_colors;
            bl_colors.shade_color = &shd_color;
            rgbacolor_t mem_color = get_pixel(x, y);
            bl_colors.mem_color   = &mem_color;
            bl_colors.combined    = NULL;

            color = process_pixel(cc_colors, bl_colors);
        }

        set_pixel(x, y, color);
        if (lft == 0) --x;
        else          ++x;
    }

    if (shade && is_cycles && xmax != 0)
    {
        *shd_red   = shd_red_temp;
        *shd_green = shd_green_temp;
        *shd_blue  = shd_blue_temp;
        *shd_alpha = shd_alpha_temp;
    }
}

void draw_scanbuffer(uint32_t* scanbuffer, edgecoeff_t* edges, shadecoeff_t* shade)
{
    uint32_t screen_y1 = (uint32_t)(scissor_border.border.YH >> 2);
    uint32_t screen_y2 = (uint32_t)(scissor_border.border.YL >> 2);

    if (curr_colorimage.format == FRMT_RGBA)
    {
        float shd_red   = 0;
        float shd_green = 0;
        float shd_blue  = 0;
        float shd_alpha = 0;

        float shd_DrDx = 0;
        float shd_DgDx = 0;
        float shd_DbDx = 0;
        float shd_DaDx = 0;

        float shd_DrDe = 0;
        float shd_DgDe = 0;
        float shd_DbDe = 0;
        float shd_DaDe = 0;

        float shd_DrDy = 0;
        float shd_DgDy = 0;
        float shd_DbDy = 0;
        float shd_DaDy = 0;

        bool is_cycles = othermodes.cycle_type == CYCLE_1 || othermodes.cycle_type == CYCLE_2;

        if (shade && is_cycles)
        {
            shd_red   = get_float_value_from_frmt((short)shade->red,   shade->red_frac,   65535.0f);
            shd_green = get_float_value_from_frmt((short)shade->green, shade->green_frac, 65535.0f);
            shd_blue  = get_float_value_from_frmt((short)shade->blue,  shade->blue_frac,  65535.0f);
            shd_alpha = get_float_value_from_frmt((short)shade->blue,  shade->blue_frac,  65535.0f);

            shd_DrDx = get_float_value_from_frmt((short)shade->DrDx, shade->DrDx_frac, 65535.0f);
            shd_DgDx = get_float_value_from_frmt((short)shade->DgDx, shade->DgDx_frac, 65535.0f);
            shd_DbDx = get_float_value_from_frmt((short)shade->DbDx, shade->DbDx_frac, 65535.0f);
            shd_DaDx = get_float_value_from_frmt((short)shade->DaDx, shade->DaDx_frac, 65535.0f);

            shd_DrDe = get_float_value_from_frmt((short)shade->DrDe, shade->DrDe_frac, 65535.0f);
            shd_DgDe = get_float_value_from_frmt((short)shade->DgDe, shade->DgDe_frac, 65535.0f);
            shd_DbDe = get_float_value_from_frmt((short)shade->DbDe, shade->DbDe_frac, 65535.0f);
            shd_DaDe = get_float_value_from_frmt((short)shade->DaDe, shade->DaDe_frac, 65535.0f);

            shd_DrDy = get_float_value_from_frmt((short)shade->DrDy, shade->DrDy_frac, 65535.0f);
            shd_DgDy = get_float_value_from_frmt((short)shade->DgDy, shade->DgDy_frac, 65535.0f);
            shd_DbDy = get_float_value_from_frmt((short)shade->DbDy, shade->DbDy_frac, 65535.0f);
            shd_DaDy = get_float_value_from_frmt((short)shade->DaDy, shade->DaDy_frac, 65535.0f);
        }

        for (size_t y = screen_y1; y < screen_y2; ++y)
        {
            uint32_t xmin = scanbuffer[(y * 2)    ];

            uint32_t xmax = scanbuffer[(y * 2) + 1];

            if (shade && is_cycles && xmax != 0)
            {
                shd_red   += shd_DrDy;
                shd_green += shd_DgDy;
                shd_blue  += shd_DbDy;
                shd_alpha += shd_DaDy;
            }

            do_x_pixel_scanbuffer(y, edges->lft, xmax, xmin, 
                                  &shd_red,  &shd_green, &shd_blue, &shd_alpha, 
                                  &shd_DrDx, &shd_DgDx,  &shd_DbDx, &shd_DaDx,
                                  &shd_DrDe, &shd_DgDe,  &shd_DbDe, &shd_DaDe, shade);
        }
    }
}

void scan_convert_line(uint32_t* scanbuffer, float xstep, float xstart, float ystart, float yend, int side)
{
    float currx = xstart;

    for (float y = ystart; y < yend; ++y)
    {
        size_t index = ((int)y * 2) + side;
        if (index > SCANBUFFER_HEIGHT * 2) break;
        scanbuffer[index] = (uint32_t)currx;
        currx += xstep;
    }
}

void scan_convert_triangle(uint32_t* scanbuffer, edgecoeff_t* edges)
{
    float xstep1 = get_float_value_from_frmt((short)edges->DxHDy, edges->DxHDy_frac, 65535.0f);
    float xstep2 = get_float_value_from_frmt((short)edges->DxLDy, edges->DxLDy_frac, 65535.0f);
    float xstep3 = get_float_value_from_frmt((short)edges->DxMDy, edges->DxMDy_frac, 65535.0f);

    scan_convert_line(scanbuffer, xstep1, (float)edges->XH, get_ten_point_two(edges->YH), get_ten_point_two(edges->YL), (int)(1 - edges->lft));
    scan_convert_line(scanbuffer, xstep2, (float)edges->XL, get_ten_point_two(edges->YM), get_ten_point_two(edges->YL), edges->lft);
    scan_convert_line(scanbuffer, xstep3, (float)edges->XM, get_ten_point_two(edges->YH), get_ten_point_two(edges->YM), edges->lft);
}

void draw_triangle(edgecoeff_t* edges, shadecoeff_t* shade, texcoeff_t* texture, zbuffercoeff_t* zbuf)
{
    uint32_t tri_scanbuffer[SCANBUFFER_HEIGHT * 2];
    memset(tri_scanbuffer, 0, (SCANBUFFER_HEIGHT * 2) * sizeof(uint32_t));

    scan_convert_triangle(tri_scanbuffer, edges);

    draw_scanbuffer(tri_scanbuffer, edges, shade);
}

void fill_rect(rect_t* rect)
{
    uint32_t rect_x1 = (uint32_t)(rect->XH >> 2);
    uint32_t rect_y1 = (uint32_t)(rect->YH >> 2);
    uint32_t rect_x2 = (uint32_t)(rect->XL >> 2) + 1;
    uint32_t rect_y2 = (uint32_t)(rect->YL >> 2);

    if (othermodes.cycle_type == CYCLE_FILL)
    {
        for (uint32_t y = rect_y1; y < rect_y2; ++y)
        {
            for (uint32_t x = rect_x1; x < rect_x2; ++x)
            {
                set_pixel_32(x, y, fill_color); // For packed 16-bit colors.
            }
        }
    }
}

void draw_tex_rect(texrect_t* tex_rect, bool flip)
{
    uint32_t rect_x1 = (tex_rect->rect.XH >> 2);
    uint32_t rect_y1 = (tex_rect->rect.YH >> 2);
    uint32_t rect_x2 = (tex_rect->rect.XL >> 2);
    uint32_t rect_y2 = (tex_rect->rect.YL >> 2);

    float S = get_ten_point_five(tex_rect->S);
    float T = get_ten_point_five(tex_rect->T);

    float DsDx = get_five_point_ten(tex_rect->DsDx);
    float DtDy = get_five_point_ten(tex_rect->DtDy);

    uint8_t index = tex_rect->tile;

    uint16_t addr0 = tiles[index].addr;
    uint16_t addr1 = (index < 7) ? tiles[index+1].addr : 0;

    for (uint32_t y = rect_y1; y < rect_y2; ++y)
    {
        S = 0;
        for (uint32_t x = rect_x1; x < rect_x2; ++x)
        {
            rgbacolor_t texel0;
            rgbacolor_t texel1;
            memset(&texel0, 0, sizeof(rgbacolor_t));
            memset(&texel1, 0, sizeof(rgbacolor_t));

            if (tiles[index].format == FRMT_RGBA)
            {
                uint32_t S0 = 0;
                uint32_t T0 = 0;

                get_tex_coords(&S0, &T0, S, T, index, flip);

                uint32_t S1 = 0;
                uint32_t T1 = 0;

                if (index < 7) get_tex_coords(&S1, &T1, S, T, index+1, flip);

                size_t tex_index0 =               (S0 + T0 * (tiles[index].sh   + 1)) * ((tiles[index].size   == BPP_32) ? 4 : 2);
                size_t tex_index1 = (index < 7) ? (S1 + T1 * (tiles[index+1].sh + 1)) * ((tiles[index+1].size == BPP_32) ? 4 : 2) : 0;

                if (tex_index0 >= 4096 || tex_index1 >= 4096) return;

                if (tiles[index].size == BPP_32)
                {
                    texel0.red   = RDP_TMEM[addr0 + tex_index0 + 0];
                    texel0.green = RDP_TMEM[addr0 + tex_index0 + 1];
                    texel0.blue  = RDP_TMEM[addr0 + tex_index0 + 2];
                    texel0.alpha = RDP_TMEM[addr0 + tex_index0 + 3];
                }
                else if (tiles[index].size == BPP_16)
                {
                    convert_16_32(&texel0, bswap_16(*(uint16_t*)&RDP_TMEM[addr0 + tex_index0]));
                }

                if (index < 7)
                {
                    if (tiles[index+1].size == BPP_32)
                    {
                        texel1.red   = RDP_TMEM[addr1 + tex_index1 + 0];
                        texel1.green = RDP_TMEM[addr1 + tex_index1 + 1];
                        texel1.blue  = RDP_TMEM[addr1 + tex_index1 + 2];
                        texel1.alpha = RDP_TMEM[addr1 + tex_index1 + 3];
                    }
                    else if (tiles[index+1].size == BPP_16)
                    {
                        convert_16_32(&texel1, bswap_16(*(uint16_t*)&RDP_TMEM[addr1 + tex_index1]));
                    }
                }

                cccolorin_t cc_colors;
                cc_colors.combined     = NULL;
                cc_colors.shade_color  = NULL;
                cc_colors.texel0_color = &texel0;
                cc_colors.texel1_color = &texel1;

                blcolorin_t bl_colors;
                bl_colors.shade_color = NULL;
                rgbacolor_t mem_color = get_pixel(x, y);
                bl_colors.mem_color   = &mem_color;
                bl_colors.combined    = NULL;

                uint32_t color = process_pixel(cc_colors, bl_colors);
                set_pixel(x, y, color);
            }
            else
            {
                puts("Texture formats other than RGBA are not supported currently.\n");
                return;
            }
            S += DsDx;
        }
        T += DtDy;
    }
}