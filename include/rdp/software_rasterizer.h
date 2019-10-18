#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "rdp.h"

void draw_triangle(edgecoeff_t* edges, shadecoeff_t* shade, texcoeff_t* tex, zbuffercoeff_t* zbuf);
void fill_rect    (rect_t* rect);
void draw_tex_rect(texrect_t* tex_rect, bool flip);