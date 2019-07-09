#include "rdp/interpreter.h"

#include "common.h"

__attribute__((__always_inline__)) static inline void undefined_inst_error(uint64_t value)
{
    fprintf(stderr, "ERROR: Unimplemented RDP command 0x%lx!  RDP PC: 0x%x, RDP Start: 0x%x, RDP End: 0x%x\n", value, bswap_32(DPC_CURRENT_REG_R), bswap_32(DPC_START_REG_RW), bswap_32(DPC_END_REG_RW));
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
    curr_colorimage.image_format = (imageformat_t)((value & 0x00E0000000000000) >> 53);
    curr_colorimage.image_size   = (bpp_t)        ((value & 0x0018000000000000) >> 51);
    curr_colorimage.image_width  = (uint16_t)     ((value & 0x000003FF00000000) >> 32);
    curr_colorimage.image_addr   = (uint32_t)      (value & 0x000000001FFFFFFF);
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

void cmd_SetCombineMode(uint64_t value)
{
    combinemode.sub_a_R_0 = (value & 0b0000000011110000000000000000000000000000000000000000000000000000) >> 52;
    combinemode.mul_R_0   = (value & 0b0000000000001111100000000000000000000000000000000000000000000000) >> 47;
    combinemode.sub_a_A_0 = (value & 0b0000000000000000011100000000000000000000000000000000000000000000) >> 44;
    combinemode.mul_A_0   = (value & 0b0000000000000000000011100000000000000000000000000000000000000000) >> 41;
    combinemode.sub_a_R_1 = (value & 0b0000000000000000000000011110000000000000000000000000000000000000) >> 37;
    combinemode.mul_R_1   = (value & 0b0000000000000000000000000001111100000000000000000000000000000000) >> 32;
    combinemode.sub_b_R_0 = (value & 0b0000000000000000000000000000000011110000000000000000000000000000) >> 28;
    combinemode.sub_b_R_1 = (value & 0b0000000000000000000000000000000000001111000000000000000000000000) >> 24;
    combinemode.sub_a_A_1 = (value & 0b0000000000000000000000000000000000000000111000000000000000000000) >> 21;
    combinemode.mul_A_1   = (value & 0b0000000000000000000000000000000000000000000111000000000000000000) >> 18;
    combinemode.add_R_0   = (value & 0b0000000000000000000000000000000000000000000000111000000000000000) >> 15;
    combinemode.sub_b_A_0 = (value & 0b0000000000000000000000000000000000000000000000000111000000000000) >> 12;
    combinemode.add_A_0   = (value & 0b0000000000000000000000000000000000000000000000000000111000000000) >> 9;
    combinemode.add_R_1   = (value & 0b0000000000000000000000000000000000000000000000000000000111000000) >> 6;
    combinemode.sub_b_A_1 = (value & 0b0000000000000000000000000000000000000000000000000000000000111000) >> 3;
    combinemode.add_A_1   = (value & 0b0000000000000000000000000000000000000000000000000000000000000111);

    add_PC(8);
}

void cmd_SetOtherModes(uint64_t value)
{
    othermodes.atomic_prim      = (value & 0b0000000010000000000000000000000000000000000000000000000000000000) > 0;
    othermodes.cycle_type       = (value & 0b0000000000110000000000000000000000000000000000000000000000000000) >> 52;
    othermodes.persp_tex_en     = (value & 0b0000000000001000000000000000000000000000000000000000000000000000) > 0;
    othermodes.detail_tex_en    = (value & 0b0000000000000100000000000000000000000000000000000000000000000000) > 0;
    othermodes.sharpen_tex_en   = (value & 0b0000000000000010000000000000000000000000000000000000000000000000) > 0;
    othermodes.tex_lod_en       = (value & 0b0000000000000001000000000000000000000000000000000000000000000000) > 0;
    othermodes.en_tlut          = (value & 0b0000000000000000100000000000000000000000000000000000000000000000) > 0;
    othermodes.tlut_type        = (value & 0b0000000000000000010000000000000000000000000000000000000000000000) >> 46;
    othermodes.sample_type      = (value & 0b0000000000000000001000000000000000000000000000000000000000000000) >> 45;
    othermodes.mid_texel        = (value & 0b0000000000000000000100000000000000000000000000000000000000000000) > 0;
    othermodes.bi_lerp_0        = (value & 0b0000000000000000000010000000000000000000000000000000000000000000) > 0;
    othermodes.bi_lerp_1        = (value & 0b0000000000000000000001000000000000000000000000000000000000000000) > 0;
    othermodes.convert_one      = (value & 0b0000000000000000000000100000000000000000000000000000000000000000) > 0;
    othermodes.key_en           = (value & 0b0000000000000000000000010000000000000000000000000000000000000000) > 0;
    othermodes.rgb_dither_sel   = (value & 0b0000000000000000000000001100000000000000000000000000000000000000) >> 38;
    othermodes.alpha_dither_sel = (value & 0b0000000000000000000000000011000000000000000000000000000000000000) >> 36;
    othermodes.b_m1a_0          = (value & 0b0000000000000000000000000000000011000000000000000000000000000000) >> 30;
    othermodes.b_m1a_1          = (value & 0b0000000000000000000000000000000000110000000000000000000000000000) >> 28;
    othermodes.b_m1b_0          = (value & 0b0000000000000000000000000000000000001100000000000000000000000000) >> 26;
    othermodes.b_m1b_1          = (value & 0b0000000000000000000000000000000000000011000000000000000000000000) >> 24;
    othermodes.b_m2a_0          = (value & 0b0000000000000000000000000000000000000000110000000000000000000000) >> 22;
    othermodes.b_m2a_1          = (value & 0b0000000000000000000000000000000000000000001100000000000000000000) >> 20;
    othermodes.b_m2b_0          = (value & 0b0000000000000000000000000000000000000000000011000000000000000000) >> 18;
    othermodes.b_m2b_1          = (value & 0b0000000000000000000000000000000000000000000000110000000000000000) >> 16;
    othermodes.force_blend      = (value & 0b0000000000000000000000000000000000000000000000000100000000000000) > 0;
    othermodes.alpha_cvg_select = (value & 0b0000000000000000000000000000000000000000000000000010000000000000) > 0;
    othermodes.cvg_times_alpha  = (value & 0b0000000000000000000000000000000000000000000000000001000000000000) > 0;
    othermodes.z_mode           = (value & 0b0000000000000000000000000000000000000000000000000000110000000000) >> 10;
    othermodes.cvg_dest         = (value & 0b0000000000000000000000000000000000000000000000000000001100000000) >> 8;
    othermodes.color_on_cvg     = (value & 0b0000000000000000000000000000000000000000000000000000000010000000) > 0;
    othermodes.image_read_en    = (value & 0b0000000000000000000000000000000000000000000000000000000001000000) > 0;
    othermodes.z_update_en      = (value & 0b0000000000000000000000000000000000000000000000000000000000100000) > 0;
    othermodes.z_compare_en     = (value & 0b0000000000000000000000000000000000000000000000000000000000010000) > 0;
    othermodes.anti_alias_en    = (value & 0b0000000000000000000000000000000000000000000000000000000000001000) > 0;
    othermodes.z_source_sel     = (value & 0b0000000000000000000000000000000000000000000000000000000000000100) > 0;
    othermodes.dither_alpha_en  = (value & 0b0000000000000000000000000000000000000000000000000000000000000010) > 0;
    othermodes.alpha_compare_en = (value & 0b0000000000000000000000000000000000000000000000000000000000000001) > 0;

    add_PC(8);
}

void cmd_SetFillColor(uint64_t value)
{
    fill_color = (uint32_t)(value & 0xFFFFFFFF);
    add_PC(8);
}

void cmd_SetScissor(uint64_t value)
{
    scissor_border.border.XH = (uint16_t)((value & 0x00FFF00000000000) >> 44);
    scissor_border.border.YH = (uint16_t)((value & 0x00000FFF00000000) >> 32);
    scissor_border.border.XL = (uint16_t)((value & 0x0000000000FFF000) >> 12);
    scissor_border.border.YL = (uint16_t) (value & 0x0000000000000FFF);

    scissor_border.f = ((value & 0x0000000002000000) > 0);
    scissor_border.o = ((value & 0x0000000001000000) > 0);
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
    uint8_t flags = (value & 0b0011111100000000000000000000000000000000000000000000000000000000) >> 56;

    size_t coeff_offset = 1;

    edgecoeff_t edges;
    edges.lft = (value & 0x0080000000000000) > 0;

    edges.YL  = (uint16_t)((value & 0x00003FFF00000000) >> 32);
    edges.YM  = (uint16_t)((value & 0x000000003FFF0000) >> 16);
    edges.YH  = (uint16_t)((value & 0x0000000000003FFF));

    uint64_t edge_coeff1 = get_coeff(coeff_offset++ * 8);
    uint64_t edge_coeff2 = get_coeff(coeff_offset++ * 8);
    uint64_t edge_coeff3 = get_coeff(coeff_offset++ * 8);

    edges.XL         = (uint16_t)((edge_coeff1 & 0xFFFF000000000000) >> 48);
    edges.XL_frac    = (uint16_t)((edge_coeff1 & 0x0000FFFF00000000) >> 32);
    edges.DxLDy      = (uint16_t)((edge_coeff1 & 0x00000000FFFF0000) >> 16);
    edges.DxLDy_frac = (uint16_t)((edge_coeff1 & 0x000000000000FFFF));

    edges.XH         = (uint16_t)((edge_coeff2 & 0xFFFF000000000000) >> 48);
    edges.XH_frac    = (uint16_t)((edge_coeff2 & 0x0000FFFF00000000) >> 32);
    edges.DxHDy      = (uint16_t)((edge_coeff2 & 0x00000000FFFF0000) >> 16);
    edges.DxHDy_frac = (uint16_t)((edge_coeff2 & 0x000000000000FFFF));

    edges.XM         = (uint16_t)((edge_coeff3 & 0xFFFF000000000000) >> 48);
    edges.XM_frac    = (uint16_t)((edge_coeff3 & 0x0000FFFF00000000) >> 32);
    edges.DxMDy      = (uint16_t)((edge_coeff3 & 0x00000000FFFF0000) >> 16);
    edges.DxMDy_frac = (uint16_t)((edge_coeff3 & 0x000000000000FFFF));

    shadecoeff_t shade;
    shadecoeff_t* shade_point = NULL;

    if ((flags & 0b0100) > 0) // Shaded
    {
        uint64_t shade_coeff1 = get_coeff(coeff_offset++ * 8);
        uint64_t shade_coeff2 = get_coeff(coeff_offset++ * 8);
        uint64_t shade_coeff3 = get_coeff(coeff_offset++ * 8);
        uint64_t shade_coeff4 = get_coeff(coeff_offset++ * 8);
        uint64_t shade_coeff5 = get_coeff(coeff_offset++ * 8);
        uint64_t shade_coeff6 = get_coeff(coeff_offset++ * 8);
        uint64_t shade_coeff7 = get_coeff(coeff_offset++ * 8);
        uint64_t shade_coeff8 = get_coeff(coeff_offset++ * 8);

        shade.red   = (uint16_t)((shade_coeff1 & 0xFFFF000000000000) >> 48);
        shade.green = (uint16_t)((shade_coeff1 & 0x0000FFFF00000000) >> 32);
        shade.blue  = (uint16_t)((shade_coeff1 & 0x00000000FFFF0000) >> 16);
        shade.alpha = (uint16_t)((shade_coeff1 & 0x000000000000FFFF));

        shade.DrDx = (uint16_t)((shade_coeff2 & 0xFFFF000000000000) >> 48);
        shade.DgDx = (uint16_t)((shade_coeff2 & 0x0000FFFF00000000) >> 32);
        shade.DbDx = (uint16_t)((shade_coeff2 & 0x00000000FFFF0000) >> 16);
        shade.DaDx = (uint16_t)((shade_coeff2 & 0x000000000000FFFF));

        shade.red_frac   = (uint16_t)((shade_coeff3 & 0xFFFF000000000000) >> 48);
        shade.green_frac = (uint16_t)((shade_coeff3 & 0x0000FFFF00000000) >> 32);
        shade.blue_frac  = (uint16_t)((shade_coeff3 & 0x00000000FFFF0000) >> 16);
        shade.alpha_frac = (uint16_t)((shade_coeff3 & 0x000000000000FFFF));

        shade.DrDx_frac = (uint16_t)((shade_coeff4 & 0xFFFF000000000000) >> 48);
        shade.DgDx_frac = (uint16_t)((shade_coeff4 & 0x0000FFFF00000000) >> 32);
        shade.DbDx_frac = (uint16_t)((shade_coeff4 & 0x00000000FFFF0000) >> 16);
        shade.DaDx_frac = (uint16_t)((shade_coeff4 & 0x000000000000FFFF));

        shade.DrDe = (uint16_t)((shade_coeff5 & 0xFFFF000000000000) >> 48);
        shade.DgDe = (uint16_t)((shade_coeff5 & 0x0000FFFF00000000) >> 32);
        shade.DbDe = (uint16_t)((shade_coeff5 & 0x00000000FFFF0000) >> 16);
        shade.DaDe = (uint16_t)((shade_coeff5 & 0x000000000000FFFF));

        shade.DrDy = (uint16_t)((shade_coeff6 & 0xFFFF000000000000) >> 48);
        shade.DgDy = (uint16_t)((shade_coeff6 & 0x0000FFFF00000000) >> 32);
        shade.DbDy = (uint16_t)((shade_coeff6 & 0x00000000FFFF0000) >> 16);
        shade.DaDy = (uint16_t)((shade_coeff6 & 0x000000000000FFFF));

        shade.DrDe_frac = (uint16_t)((shade_coeff7 & 0xFFFF000000000000) >> 48);
        shade.DgDe_frac = (uint16_t)((shade_coeff7 & 0x0000FFFF00000000) >> 32);
        shade.DbDe_frac = (uint16_t)((shade_coeff7 & 0x00000000FFFF0000) >> 16);
        shade.DaDe_frac = (uint16_t)((shade_coeff7 & 0x000000000000FFFF));

        shade.DrDy_frac = (uint16_t)((shade_coeff8 & 0xFFFF000000000000) >> 48);
        shade.DgDy_frac = (uint16_t)((shade_coeff8 & 0x0000FFFF00000000) >> 32);
        shade.DbDy_frac = (uint16_t)((shade_coeff8 & 0x00000000FFFF0000) >> 16);
        shade.DaDy_frac = (uint16_t)((shade_coeff8 & 0x000000000000FFFF));

        shade_point = &shade;
        add_PC(64);
    }

    draw_triangle(&edges, shade_point, NULL, NULL);

    add_PC(32);
}