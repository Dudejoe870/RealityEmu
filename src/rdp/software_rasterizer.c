#include "software_rasterizer.h"

#include <stddef.h>
#include <stdio.h>
#include <byteswap.h>
#include <string.h>

#include "../r4300/mem.h"

#define SCANBUFFER_HEIGHT 480

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

__attribute__((__always_inline__)) static inline void set_pixel(uint32_t x, uint32_t y, uint32_t packed_color, uint32_t SX1, uint32_t SY1, uint32_t SX2, uint32_t SY2)
{
    if ((x < SX1 || y < SY1) || (x > SX2 || y > SY2)) return;

    uint32_t index = x + y * (curr_colorimage.image_width+1);
    if (curr_colorimage.image_size == BPP_16)
        index >>= 1;
    uint32_t* framebuffer = get_real_memory_loc(curr_colorimage.image_addr);
    framebuffer[index] = bswap_32(packed_color);
}

void draw_fill_scanbuffer(uint32_t* scanbuffer)
{
    uint32_t screen_x1 = (uint32_t)(scissor_border.border.XH >> 2);
    uint32_t screen_y1 = (uint32_t)(scissor_border.border.YH >> 2);
    uint32_t screen_x2 = (uint32_t)(scissor_border.border.XL >> 2);
    uint32_t screen_y2 = (uint32_t)(scissor_border.border.YL >> 2);

    if (curr_colorimage.image_format == FRMT_RGBA)
    {
        for (size_t y = screen_y1; y < screen_y2; ++y)
        {
            uint32_t xmin = scanbuffer[(y * 2)    ];
            uint32_t xmax = scanbuffer[(y * 2) + 1];
            
            for (size_t x = xmin; x < xmax; ++x)
                set_pixel(x, y, fill_color, screen_x1, screen_y1, screen_x2, screen_y2);
        }
    }
    else
    {
        puts("Fill drawing Scanbuffers isn't supported in any other mode other than RGBA currently.");
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

    scan_convert_triangle(tri_scanbuffer, edges);

    draw_fill_scanbuffer(tri_scanbuffer);
}

void fill_rect(rect_t* rect)
{
    uint32_t rect_x1 = (uint32_t)(rect->XH >> 2);
    uint32_t rect_y1 = (uint32_t)(rect->YH >> 2);
    uint32_t rect_x2 = (uint32_t)(rect->XL >> 2) + 1;
    uint32_t rect_y2 = (uint32_t)(rect->YL >> 2) + 1;

    uint32_t rect_scanbuffer[SCANBUFFER_HEIGHT * 2];
    memset(rect_scanbuffer, 0, (SCANBUFFER_HEIGHT * 2) * sizeof(uint32_t));

    for (uint32_t y = rect_y1; y < rect_y2; ++y)
    {
        rect_scanbuffer[(y * 2)    ] = rect_x1;
        rect_scanbuffer[(y * 2) + 1] = rect_x2;
    }

    draw_fill_scanbuffer(rect_scanbuffer);
}