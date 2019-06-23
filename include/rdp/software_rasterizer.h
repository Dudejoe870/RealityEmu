#pragma once

#include <stdint.h>

#include "rdp.h"

void draw_triangle(edgecoeff_t* Edges, shadecoeff_t* Shade, texcoeff_t* Texture, zbuffercoeff_t* ZBuf);
void fill_rect    (rect_t* rect);