#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    Frmt_RGBA      = 0,
    Frmt_YUV       = 1,
    Frmt_ColorIndx = 2,
    Frmt_IA        = 3,
    Frmt_I         = 4,
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
    uint32_t      ImageAddr;
    imageformat_t ImageFormat;
    bpp_t         ImageSize;
    uint16_t      ImageWidth;
} image_t;

image_t  currColorImage;
image_t  currTexImage;
uint32_t currZImageAddr;

typedef struct
{
    uint16_t XH, YH, XL, YL;
} rect_t;

typedef struct
{
    rect_t  Rect;
    uint8_t Tile;

    uint16_t S, T;
    uint16_t DsDx, DtDy;
} texrect_t;

typedef struct
{  
    rect_t Border;
    bool f, o;
} scissorborder_t;

scissorborder_t ScissorBorder;

typedef struct
{
    bool lft;
    uint8_t Level;
    uint8_t Tile;
    
    uint16_t YL, YM, YH;

    uint16_t XL, XLFrac;
    uint16_t DxLDy, DxLDyFrac;

    uint16_t XH, XHFrac;
    uint16_t DxHDy, DxHDyFrac;

    uint16_t XM, XMFrac;
    uint16_t DxMDy, DxMDyFrac;
} edgecoeff_t;

typedef struct
{
    uint16_t Red, Green, Blue, Alpha;

    uint16_t DrDx, DgDx, DbDx, DaDx;
    
    uint16_t RedFrac, GreenFrac, BlueFrac, AlphaFrac;

    uint16_t DrDxFrac, DgDxFrac, DbDxFrac, DaDxFrac;

    uint16_t DrDe, DgDe, DbDe, DaDe;
    
    uint16_t DrDy, DgDy, DbDy, DaDy;
    
    uint16_t DrDeFrac, DgDeFrac, DbDeFrac, DaDeFrac;
    
    uint16_t DrDyFrac, DgDyFrac, DbDyFrac, DaDyFrac;
} shadecoeff_t;

typedef struct
{
    uint16_t S, T, W;

    uint16_t DsDx, DtDx, DwDx;

    uint16_t SFrac, TFrac, WFrac;

    uint16_t DsDxFrac, DtDxFrac, DwDxFrac;

    uint16_t DsDe, DtDe, DwDe;

    uint16_t DsDy, DtDy, DwDy;

    uint16_t DsDeFrac, DtDeFrac, DwDeFrac;

    uint16_t DsDyFrac, DtDyFrac, DwDyFrac;
} texcoeff_t;

typedef struct
{
    uint16_t Z, ZFrac;
    uint16_t DzDx, DzDxFrac;

    uint16_t DzDe, DzDeFrac;
    uint16_t DzDy, DzDyFrac;
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

combinemode_t currCombineMode;

typedef enum
{
    Cycle_1    = 0,
    Cycle_2    = 1,
    Cycle_Copy = 2,
    Cycle_Fill = 3
} cycletype_t;

typedef enum
{
    TLUT_RGBA5551 = 0,
    TLUT_IA88     = 1
} tluttype_t;

typedef enum
{
    Sample_1x1 = 0,
    Sample_2x2 = 1
} sampletype_t;

typedef enum
{
    RgbDither_MagicSquareMatrix = 0,
    RgbDither_BayerMatrix       = 1,
    RgbDither_Noise             = 2,
    RgbDither_None              = 3
} rgbdither_t;

typedef enum
{
    AlphaDither_Pattern  = 0,
    AlphaDither_NotPattern = 1,
    AlphaDither_Noise    = 2,
    AlphaDither_None     = 3
} alphadither_t;

typedef enum
{
    ZMode_Opaque           = 0,
    ZMode_Interpenetrating = 1,
    ZMode_Transparent      = 2,
    ZMode_Decal            = 3
} zmode_t;

typedef enum
{
    Clamp = 0,
    Wrap  = 1,
    Zap   = 2,
    Save  = 3
} cvgdest_t;

typedef struct
{
    bool AtomicPrim;

    cycletype_t CycleType;

    bool PerspTexEn;
    bool DetailTexEn;
    bool SharpenTexEn;
    bool TexLodEn;
    bool EnTlut;

    tluttype_t   TlutType;
    sampletype_t SampleType;
    
    bool MidTexel;
    bool BiLerp0;
    bool BiLerp1;
    bool ConvertOne;
    bool KeyEn;

    rgbdither_t   RgbDitherSel;
    alphadither_t AlphaDitherSel;

    uint8_t B_M1A_0;
    uint8_t B_M1A_1;
    uint8_t B_M1B_0;
    uint8_t B_M1B_1;

    uint8_t B_M2A_0;
    uint8_t B_M2A_1;
    uint8_t B_M2B_0;
    uint8_t B_M2B_1;

    bool ForceBlend;
    bool AlphaCvgSelect;
    bool CvgTimesAlpha;

    zmode_t   ZMode;
    cvgdest_t CvgDest;

    bool ColorOnCvg;
    bool ImageReadEn;
    bool ZUpdateEn;
    bool ZCompareEn;
    bool AntiAliasEn;
    bool ZSourceSel;
    bool DitherAlphaEn;
    bool AlphaCompareEn;
} othermodes_t;

othermodes_t OtherModes;

typedef struct
{
    uint8_t Red, Green, Blue, Alpha;
} rgbacolor_t;

rgbacolor_t currEnvColor;
rgbacolor_t currBlendColor;
rgbacolor_t currFogColor;

typedef struct
{
    uint8_t PrimMinLevel, PrimLevelFrac;
    rgbacolor_t Color;
} primcolor_t;

primcolor_t currPrimColor;

uint32_t FillColor;

typedef struct
{
    uint16_t PrimZ, PrimDeltaZ;
} primdepth_t;

primdepth_t currPrimDepth;

typedef struct
{
    uint16_t K0, K1, K2, K3, K4, K5;
} convert_t;

convert_t currConvert;

typedef struct
{
    uint16_t WidthR;
    uint8_t  CenterR;
    uint8_t  ScaleR;
} keyr_t;

keyr_t currKeyR;

typedef struct
{
    uint16_t WidthG, WidthB;

    uint8_t CenterG, ScaleG;
    uint8_t CenterB, ScaleB;
} keygb_t;

keygb_t currKeyGB;

bool ShouldRun;

void RDPInit  (void);
void RDPWakeUp(void);