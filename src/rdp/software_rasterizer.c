#include "software_rasterizer.h"

#include <stddef.h>
#include <stdio.h>
#include <byteswap.h>
#include <string.h>

#include "../r4300/mem.h"

#define SCANBUFFER_HEIGHT 480

static inline float GetFloatValueFromFrmt(uint32_t Int, uint32_t Dec, float DecMax)
{
    return ((int)Int) + ((float)Dec / DecMax);
}

static inline float GetTenPointTwo(uint16_t Value)
{
    return GetFloatValueFromFrmt(Value >> 2, Value & 0x3, 3.0f);
}

static inline float GetTenPointFive(uint16_t Value)
{
    return GetFloatValueFromFrmt(Value >> 5, Value & 0x1F, 31.0f);
}

static inline float GetFivePointTen(uint16_t Value)
{
    return GetFloatValueFromFrmt(Value >> 10, Value & 0x3FF, 1024.0f);
}

static inline void SetPixel(uint32_t x, uint32_t y, uint32_t PackedColor, uint32_t SX1, uint32_t SY1, uint32_t SX2, uint32_t SY2)
{
    if ((x < SX1 || y < SY1) || (x > SX2 || y > SY2)) return;

    uint32_t Index = x + y * (currColorImage.ImageWidth+1);
    if (currColorImage.ImageSize == BPP_16)
        Index >>= 1;
    uint32_t* Framebuffer = GetRealMemoryLoc(currColorImage.ImageAddr);
    Framebuffer[Index] = bswap_32(PackedColor);
}

void DrawFillScanbuffer(uint32_t* Scanbuffer)
{
    uint32_t ScreenX1 = (uint32_t)(ScissorBorder.Border.XH >> 2);
    uint32_t ScreenY1 = (uint32_t)(ScissorBorder.Border.YH >> 2);
    uint32_t ScreenX2 = (uint32_t)(ScissorBorder.Border.XL >> 2);
    uint32_t ScreenY2 = (uint32_t)(ScissorBorder.Border.YL >> 2);

    if (currColorImage.ImageFormat == Frmt_RGBA)
    {
        for (size_t y = ScreenY1; y < ScreenY2; ++y)
        {
            uint32_t xMin = Scanbuffer[(y * 2)    ];
            uint32_t xMax = Scanbuffer[(y * 2) + 1];
            
            for (size_t x = xMin; x < xMax; ++x)
                SetPixel(x, y, FillColor, ScreenX1, ScreenY1, ScreenX2, ScreenY2);
        }
    }
    else
    {
        puts("Fill drawing Scanbuffers isn't supported in any other mode other than RGBA currently.");
    }
}

void ScanConvertLine(uint32_t* Scanbuffer, float XStep, uint32_t XStart, uint32_t YStart, uint32_t YEnd, int Side)
{
    float CurrX = XStart;

    for (uint32_t y = YStart; y < YEnd; ++y)
    {
        size_t Index = (y * 2) + Side;
        if (Index > SCANBUFFER_HEIGHT * 2) break;
        Scanbuffer[Index] = (uint32_t)CurrX;
        CurrX += XStep;
    }
}

void ScanConvertTriangle(uint32_t* Scanbuffer, edgecoeff_t* Edges)
{
    float XStep1 = GetFloatValueFromFrmt((short)Edges->DxHDy, Edges->DxHDyFrac, 65535.0f);
    float XStep2 = GetFloatValueFromFrmt((short)Edges->DxLDy, Edges->DxLDyFrac, 65535.0f);
    float XStep3 = GetFloatValueFromFrmt((short)Edges->DxMDy, Edges->DxMDyFrac, 65535.0f);

    int Dir = (int)Edges->lft;

    ScanConvertLine(Scanbuffer, XStep1, Edges->XH, Edges->YH >> 2, Edges->YL >> 2, (int)(1 - Dir));
    ScanConvertLine(Scanbuffer, XStep2, Edges->XL, Edges->YM >> 2, Edges->YL >> 2, Dir);
    ScanConvertLine(Scanbuffer, XStep3, Edges->XM, Edges->YH >> 2, Edges->YM >> 2, Dir);
}

void DrawTriangle(edgecoeff_t* Edges, shadecoeff_t* Shade, texcoeff_t* Texture, zbuffercoeff_t* ZBuf)
{
    uint32_t TriScanbuffer[SCANBUFFER_HEIGHT * 2];
    memset(TriScanbuffer, 0, (SCANBUFFER_HEIGHT * 2) * sizeof(uint32_t));

    ScanConvertTriangle(TriScanbuffer, Edges);

    DrawFillScanbuffer(TriScanbuffer);
}

void FillRect(rect_t* Rect)
{
    uint32_t RectX1 = (uint32_t)(Rect->XH >> 2);
    uint32_t RectY1 = (uint32_t)(Rect->YH >> 2);
    uint32_t RectX2 = (uint32_t)(Rect->XL >> 2) + 1;
    uint32_t RectY2 = (uint32_t)(Rect->YL >> 2) + 1;

    uint32_t RectScanbuffer[SCANBUFFER_HEIGHT * 2];
    memset(RectScanbuffer, 0, (SCANBUFFER_HEIGHT * 2) * sizeof(uint32_t));

    for (uint32_t y = RectY1; y < RectY2; ++y)
    {
        RectScanbuffer[(y * 2)    ] = RectX1;
        RectScanbuffer[(y * 2) + 1] = RectX2;
    }

    DrawFillScanbuffer(RectScanbuffer);
}