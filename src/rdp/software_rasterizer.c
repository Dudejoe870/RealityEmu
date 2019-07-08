#include "rdp/software_rasterizer.h"

#include "common.h"

#define SCANBUFFER_HEIGHT 480
#define MAX_WIDTH 640

__attribute__((__always_inline__)) static inline float get_float_value_from_frmt(uint32_t integer, uint32_t decimal, float decimal_max)
{
    return ((int)integer) + ((float)decimal / decimal_max);
}

__attribute__((__always_inline__)) static inline float get_ten_point_two(uint16_t value)
{
    return get_float_value_from_frmt(value >> 2, value & 0x3, 3.0f);
}

__attribute__((__always_inline__)) static inline float get_ten_point_five(uint16_t value)
{
    return get_float_value_from_frmt(value >> 5, value & 0x1F, 31.0f);
}

__attribute__((__always_inline__)) static inline float get_five_point_ten(uint16_t value)
{
    return get_float_value_from_frmt(value >> 10, value & 0x3FF, 1024.0f);
}

__attribute__((__always_inline__)) static inline uint32_t get_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (curr_colorimage.image_size == BPP_32)
        return (r << 24) | (g << 16) | (b << 8) | a;
    else if (curr_colorimage.image_size == BPP_16)
        return (((r & 0b11111) << 11) | ((g & 0b11111) << 6) | ((b & 0b11111) << 1) | (a > 0)) 
            | ((((r & 0b11111) << 11) | ((g & 0b11111) << 6) | ((b & 0b11111) << 1) | (a > 0)) << 16);
    return 0;
}

__attribute__((__always_inline__)) static inline void set_pixel(uint32_t x, uint32_t y, uint32_t packed_color)
{
    uint32_t screen_x1 = (uint32_t)(scissor_border.border.XH >> 2);
    uint32_t screen_y1 = (uint32_t)(scissor_border.border.YH >> 2);
    uint32_t screen_x2 = (uint32_t)(scissor_border.border.XL >> 2);
    uint32_t screen_y2 = (uint32_t)(scissor_border.border.YL >> 2);

    if ((x < screen_x1 || y < screen_y1) || (x > screen_x2 || y > screen_y2)) return;

    uint32_t index = x + y * (curr_colorimage.image_width+1);
    if (curr_colorimage.image_size == BPP_16)
        index >>= 1;
    uint32_t* framebuffer = get_real_memory_loc(curr_colorimage.image_addr);
    framebuffer[index] = bswap_32(packed_color);
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
            if (shade)
            {
                bool edge_cond = false;
                if (lft == 0) edge_cond = x == xmax;
                else          edge_cond = x == xmin;
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

                color = get_color((uint8_t)(*shd_red), (uint8_t)(*shd_green), (uint8_t)(*shd_blue), (uint8_t)(*shd_alpha));
            }
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

    if (curr_colorimage.image_format == FRMT_RGBA)
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
                                  &shd_red, &shd_green, &shd_blue, &shd_alpha, 
                                  &shd_DrDx, &shd_DgDx, &shd_DbDx, &shd_DaDx,
                                  &shd_DrDe, &shd_DgDe, &shd_DbDe, &shd_DaDe, shade);
        }
    }
}

void scan_convert_line(uint32_t* scanbuffer, float xstep, uint32_t xstart, uint32_t ystart, uint32_t yend, int side)
{
    float currx = xstart;

    for (uint32_t y = ystart; y < yend; ++y)
    {
        size_t index = (y * 2) + side;
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

    scan_convert_line(scanbuffer, xstep1, edges->XH, edges->YH >> 2, edges->YL >> 2, (int)(1 - edges->lft));
    scan_convert_line(scanbuffer, xstep2, edges->XL, edges->YM >> 2, edges->YL >> 2, edges->lft);
    scan_convert_line(scanbuffer, xstep3, edges->XM, edges->YH >> 2, edges->YM >> 2, edges->lft);
}

void draw_triangle(edgecoeff_t* edges, shadecoeff_t* shade, texcoeff_t* texture, zbuffercoeff_t* zbuf)
{
    uint32_t tri_scanbuffer[SCANBUFFER_HEIGHT * 2];
    memset(tri_scanbuffer, 0, (SCANBUFFER_HEIGHT * 2) * sizeof(uint32_t));

    rgbacolor_t shade_edge_table[MAX_WIDTH][SCANBUFFER_HEIGHT];
    memset(&shade_edge_table, 0, sizeof(shade_edge_table));

    scan_convert_triangle(tri_scanbuffer, edges);

    draw_scanbuffer(tri_scanbuffer, edges, shade);
}

void fill_rect(rect_t* rect)
{
    uint32_t rect_x1 = (uint32_t)(rect->XH >> 2);
    uint32_t rect_y1 = (uint32_t)(rect->YH >> 2);
    uint32_t rect_x2 = (uint32_t)(rect->XL >> 2) + 1;
    uint32_t rect_y2 = (uint32_t)(rect->YL >> 2) + 1;

    if (othermodes.cycle_type == CYCLE_FILL)
    {
        for (uint32_t y = rect_y1; y < rect_y2; ++y)
        {
            for (uint32_t x = rect_x1; x < rect_x2; ++x)
            {
                set_pixel(x, y, fill_color);
            }
        }
    }
}