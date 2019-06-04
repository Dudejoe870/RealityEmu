#pragma once

#include <stdint.h>

#include "rdp.h"

void DrawTriangle(edgecoeff_t* Edges, shadecoeff_t* Shade, texcoeff_t* Texture, zbuffercoeff_t* ZBuf);
void FillRect    (rect_t* Rect);