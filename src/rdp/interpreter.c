#include "interpreter.h"

#include "rdp.h"
#include "software_rasterizer.h"
#include "../r4300/cpu.h"
#include "cmdtable.h"
#include "../r4300/mem.h"
#include "../r4300/mi.h"

#include <byteswap.h>
#include <stdio.h>

__attribute__((__always_inline__)) static inline void undefined_inst_error(uint64_t value)
{
    fprintf(stderr, "ERROR: Unimplemented RDP command 0x%lx!  RDP PC: 0x%x\n", value, bswap_32(DPC_CURRENT_REG_R));
    is_running = false;
}

__attribute__((__always_inline__)) static inline uint64_t get_coeff(uint8_t offset)
{
    return ((bswap_32(DPC_STATUS_REG_R) & 1) > 0) 
           ? bswap_64(((uint64_t*)SP_DMEM_RW)[bswap_32(DPC_CURRENT_REG_R & 0xFFF) + offset]) 
           : read_uint64(bswap_32(DPC_CURRENT_REG_R) + offset);
}

__attribute__((__always_inline__)) static inline void add_PC(uint8_t ToAdd)
{
    DPC_CURRENT_REG_R = bswap_32(bswap_32(DPC_CURRENT_REG_R) + ToAdd);
}

__attribute__((__always_inline__)) static inline void set_PC(uint32_t ToSet)
{
    DPC_CURRENT_REG_R = bswap_32(ToSet);
}

void RDP_step(void)
{
    uint64_t inst = ((bswap_32(DPC_STATUS_REG_R) & 1) > 0) 
                    ? bswap_64(((uint64_t*)SP_DMEM_RW)[bswap_32(DPC_CURRENT_REG_R & 0xFFF)]) 
                    : read_uint64(bswap_32(DPC_CURRENT_REG_R));
    cmd_t command = CMDtable[(inst & 0x3F00000000000000) >> 56];

    if (command.interpret == NULL && inst != 0) 
    {
        undefined_inst_error(inst);
        return;
    }

    if (inst != 0) command.interpret(inst);
    else add_PC(8);

    if (DPC_CURRENT_REG_R == DPC_END_REG_RW)
        should_run = false;
}

void cmd_SetColorImage(uint64_t value)
{
    image_t image;
    image.image_format = (imageformat_t)((value & 0x00E0000000000000) >> 53);
    image.image_size   = (bpp_t)        ((value & 0x0018000000000000) >> 51);
    image.image_width  = (uint16_t)     ((value & 0x000003FF00000000) >> 32);
    image.image_addr   = (uint32_t)      (value & 0x000000001FFFFFFF);

    curr_colorimage = image;
    add_PC(8);
}

void cmd_FillRectangle(uint64_t value)
{
    rect_t rect;
    rect.XL = (uint16_t)((value & 0x00FFF00000000000) >> 44);
    rect.YL = (uint16_t)((value & 0x00000FFF00000000) >> 32);
    rect.XH = (uint16_t)((value & 0x0000000000FFF000) >> 12);
    rect.YH = (uint16_t) (value & 0x0000000000000FFF);

    fill_rect(&rect);
    add_PC(8);
}

void cmd_SetOtherModes(uint64_t value)
{
    // Stubbed for now.
    add_PC(8);
}

void cmd_SetFillColor(uint64_t value)
{
    fill_color = (uint32_t)(value & 0xFFFFFFFF);
    add_PC(8);
}

void cmd_SetScissor(uint64_t value)
{
    scissorborder_t scissor;
    scissor.border.XH = (uint16_t)((value & 0x00FFF00000000000) >> 44);
    scissor.border.YH = (uint16_t)((value & 0x00000FFF00000000) >> 32);
    scissor.border.XL = (uint16_t)((value & 0x0000000000FFF000) >> 12);
    scissor.border.YL = (uint16_t) (value & 0x0000000000000FFF);

    scissor.f = ((value & 0x0000000002000000) > 0);
    scissor.o = ((value & 0x0000000001000000) > 0);

    scissor_border = scissor;
    add_PC(8);
}

void cmd_SyncFull(uint64_t value)
{
    invoke_mi_interrupt(MI_INTR_DP);
    add_PC(8);
}

void cmd_SyncPipe(uint64_t value)
{
    // Stubbed.
    add_PC(8);
}

void cmd_Triangle(uint64_t value)
{
    edgecoeff_t edges;
    edges.lft = (value & 0x0080000000000000) > 0;

    edges.YL  = (uint16_t)((value & 0x00003FFF00000000) >> 32);
    edges.YM  = (uint16_t)((value & 0x000000003FFF0000) >> 16);
    edges.YH  = (uint16_t)((value & 0x0000000000003FFF));

    uint64_t edge_coeff1 = get_coeff(8);
    uint64_t edge_coeff2 = get_coeff(16);
    uint64_t edge_coeff3 = get_coeff(24);

    edges.XL        = (uint16_t)((edge_coeff1 & 0xFFFF000000000000) >> 48);
    edges.XL_frac    = (uint16_t)((edge_coeff1 & 0x0000FFFF00000000) >> 32);
    edges.DxLDy     = (uint16_t)((edge_coeff1 & 0x00000000FFFF0000) >> 16);
    edges.DxLDy_frac = (uint16_t)((edge_coeff1 & 0x000000000000FFFF));

    edges.XH        = (uint16_t)((edge_coeff2 & 0xFFFF000000000000) >> 48);
    edges.XH_frac    = (uint16_t)((edge_coeff2 & 0x0000FFFF00000000) >> 32);
    edges.DxHDy     = (uint16_t)((edge_coeff2 & 0x00000000FFFF0000) >> 16);
    edges.DxHDy_frac = (uint16_t)((edge_coeff2 & 0x000000000000FFFF));

    edges.XM        = (uint16_t)((edge_coeff3 & 0xFFFF000000000000) >> 48);
    edges.XM_frac    = (uint16_t)((edge_coeff3 & 0x0000FFFF00000000) >> 32);
    edges.DxMDy     = (uint16_t)((edge_coeff3 & 0x00000000FFFF0000) >> 16);
    edges.DxMDy_frac = (uint16_t)((edge_coeff3 & 0x000000000000FFFF));

    draw_triangle(&edges, NULL, NULL, NULL);

    add_PC(32);
}