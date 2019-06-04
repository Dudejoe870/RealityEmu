#include "interpreter.h"

#include "rdp.h"
#include "software_rasterizer.h"
#include "../r4300/cpu.h"
#include "cmdtable.h"
#include "../r4300/mem.h"
#include "../r4300/mi.h"

#include <byteswap.h>
#include <stdio.h>

static inline void UndefinedInstError(uint64_t Value)
{
    fprintf(stderr, "ERROR: Unimplemented RDP Command 0x%lx!  RDP PC: 0x%x\n", Value, bswap_32(DPC_CURRENT_REG_R));
    IsRunning = false;
}

static inline uint64_t GetCoeff(uint8_t Offset)
{
    return ((bswap_32(DPC_STATUS_REG_R) & 1) > 0) 
           ? bswap_64(((uint64_t*)SP_DMEM_RW)[bswap_32(DPC_CURRENT_REG_R & 0xFFF) + Offset]) 
           : ReadUInt64(bswap_32(DPC_CURRENT_REG_R) + Offset);
}

static inline void AddPC(uint8_t ToAdd)
{
    DPC_CURRENT_REG_R = bswap_32(bswap_32(DPC_CURRENT_REG_R) + ToAdd);
}

static inline void SetPC(uint32_t ToSet)
{
    DPC_CURRENT_REG_R = bswap_32(ToSet);
}

void RDPStep(void)
{
    uint64_t Inst = ((bswap_32(DPC_STATUS_REG_R) & 1) > 0) 
                    ? bswap_64(((uint64_t*)SP_DMEM_RW)[bswap_32(DPC_CURRENT_REG_R & 0xFFF)]) 
                    : ReadUInt64(bswap_32(DPC_CURRENT_REG_R));
    cmd_t Command = CMDTable[(Inst & 0x3F00000000000000) >> 56];

    if (Command.Interpret == NULL && Inst != 0) 
    {
        UndefinedInstError(Inst);
        return;
    }

    if (Inst != 0) Command.Interpret(Inst);
    else AddPC(8);

    if (DPC_CURRENT_REG_R == DPC_END_REG_RW)
        ShouldRun = false;
}

void SetColorImage(uint64_t Value)
{
    image_t Image;
    Image.ImageFormat = (imageformat_t)((Value & 0x00E0000000000000) >> 53);
    Image.ImageSize   = (bpp_t)        ((Value & 0x0018000000000000) >> 51);
    Image.ImageWidth  = (uint16_t)     ((Value & 0x000003FF00000000) >> 32);
    Image.ImageAddr   = (uint32_t)      (Value & 0x000000001FFFFFFF);

    currColorImage = Image;
    AddPC(8);
}

void FillRectangle(uint64_t Value)
{
    rect_t Rect;
    Rect.XL = (uint16_t)((Value & 0x00FFF00000000000) >> 44);
    Rect.YL = (uint16_t)((Value & 0x00000FFF00000000) >> 32);
    Rect.XH = (uint16_t)((Value & 0x0000000000FFF000) >> 12);
    Rect.YH = (uint16_t) (Value & 0x0000000000000FFF);

    FillRect(&Rect);
    AddPC(8);
}

void SetOtherModes(uint64_t Value)
{
    // Stubbed for now.
    AddPC(8);
}

void SetFillColor(uint64_t Value)
{
    FillColor = (uint32_t)(Value & 0xFFFFFFFF);
    AddPC(8);
}

void SetScissor(uint64_t Value)
{
    scissorborder_t Scissor;
    Scissor.Border.XH = (uint16_t)((Value & 0x00FFF00000000000) >> 44);
    Scissor.Border.YH = (uint16_t)((Value & 0x00000FFF00000000) >> 32);
    Scissor.Border.XL = (uint16_t)((Value & 0x0000000000FFF000) >> 12);
    Scissor.Border.YL = (uint16_t) (Value & 0x0000000000000FFF);

    Scissor.f = ((Value & 0x0000000002000000) > 0);
    Scissor.o = ((Value & 0x0000000001000000) > 0);

    ScissorBorder = Scissor;
    AddPC(8);
}

void SyncFull(uint64_t Value)
{
    InvokeMIInterrupt(MI_INTR_DP);
    AddPC(8);
}

void SyncPipe(uint64_t Value)
{
    // Stubbed.
    AddPC(8);
}

void Triangle(uint64_t Value)
{
    edgecoeff_t Edges;
    Edges.lft = (Value & 0x0080000000000000) > 0;

    Edges.YL  = (uint16_t)((Value & 0x00003FFF00000000) >> 32);
    Edges.YM  = (uint16_t)((Value & 0x000000003FFF0000) >> 16);
    Edges.YH  = (uint16_t)((Value & 0x0000000000003FFF));

    uint64_t EdgeCoeff1 = GetCoeff(8);
    uint64_t EdgeCoeff2 = GetCoeff(16);
    uint64_t EdgeCoeff3 = GetCoeff(24);

    Edges.XL        = (uint16_t)((EdgeCoeff1 & 0xFFFF000000000000) >> 48);
    Edges.XLFrac    = (uint16_t)((EdgeCoeff1 & 0x0000FFFF00000000) >> 32);
    Edges.DxLDy     = (uint16_t)((EdgeCoeff1 & 0x00000000FFFF0000) >> 16);
    Edges.DxLDyFrac = (uint16_t)((EdgeCoeff1 & 0x000000000000FFFF));

    Edges.XH        = (uint16_t)((EdgeCoeff2 & 0xFFFF000000000000) >> 48);
    Edges.XHFrac    = (uint16_t)((EdgeCoeff2 & 0x0000FFFF00000000) >> 32);
    Edges.DxHDy     = (uint16_t)((EdgeCoeff2 & 0x00000000FFFF0000) >> 16);
    Edges.DxHDyFrac = (uint16_t)((EdgeCoeff2 & 0x000000000000FFFF));

    Edges.XM        = (uint16_t)((EdgeCoeff3 & 0xFFFF000000000000) >> 48);
    Edges.XMFrac    = (uint16_t)((EdgeCoeff3 & 0x0000FFFF00000000) >> 32);
    Edges.DxMDy     = (uint16_t)((EdgeCoeff3 & 0x00000000FFFF0000) >> 16);
    Edges.DxMDyFrac = (uint16_t)((EdgeCoeff3 & 0x000000000000FFFF));

    DrawTriangle(&Edges, NULL, NULL, NULL);

    AddPC(32);
}