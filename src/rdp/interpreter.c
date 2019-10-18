#include "rdp/interpreter.h"

#include "common.h"

__attribute__((__always_inline__)) static inline void RDP_undefined_inst_error(uint64_t value)
{
    fprintf(stderr, "ERROR: Unimplemented RDP command 0x%lx!  RDP PC: 0x%x, RDP Start: 0x%x, RDP End: 0x%x\n", value, bswap_32(DPC_CURRENT_REG_R), bswap_32(DPC_START_REG_RW), bswap_32(DPC_END_REG_RW));
    is_running = false;
}

__attribute__((__always_inline__)) static inline uint64_t get_coeff(uint16_t offset)
{
    return ((bswap_32(DPC_STATUS_REG_R) & 1) > 0) 
           ? read_uint64(0x04000000 | (bswap_32(DPC_CURRENT_REG_R) + offset))
           : read_uint64(bswap_32(DPC_CURRENT_REG_R) + offset);
}

__attribute__((__always_inline__)) static inline void add_PC(uint8_t value)
{
    DPC_CURRENT_REG_R = bswap_32(bswap_32(DPC_CURRENT_REG_R) + value);
}

__attribute__((__always_inline__)) static inline void set_PC(uint32_t value)
{
    DPC_CURRENT_REG_R = bswap_32(value);
}

void RDP_step(void)
{
    uint64_t inst = ((bswap_32(DPC_STATUS_REG_R) & 1) > 0) 
                    ? read_uint64(0x04000000 | bswap_32(DPC_CURRENT_REG_R))
                    : read_uint64(bswap_32(DPC_CURRENT_REG_R));
    cmd_t command = CMDtable[(inst & 0x3F00000000000000) >> 56];

    if (command.interpret == NULL && inst != 0) 
    {
        RDP_undefined_inst_error(inst);
        return;
    }

    if (inst != 0) command.interpret(inst);
    else add_PC(8);

    if (DPC_CURRENT_REG_R == DPC_END_REG_RW)
        should_run = false;
}

void cmd_SetColorImage(uint64_t value)
{
    curr_colorimage.format = (value & 0x00E0000000000000) >> 53;
    curr_colorimage.size   = (value & 0x0018000000000000) >> 51;
    curr_colorimage.width  = (value & 0x000003FF00000000) >> 32;
    curr_colorimage.addr   = (value & 0x000000001FFFFFFF);
    add_PC(8);
}

void cmd_SetTextureImage(uint64_t value)
{
    curr_teximage.format = (value & 0b0000000011100000000000000000000000000000000000000000000000000000) >> 53;
    curr_teximage.size   = (value & 0b0000000000011000000000000000000000000000000000000000000000000000) >> 51;
    curr_teximage.width  = (value & 0b0000000000000000000000111111111100000000000000000000000000000000) >> 32;
    curr_teximage.addr   = (value & 0b0000000000000000000000000000000000000011111111111111111111111111);
    add_PC(8);
}

void cmd_SetTile(uint64_t value)
{
    uint8_t index        = (value & 0b0000000000000000000000000000000000000111000000000000000000000000) >> 24;
    tiles[index].format  = (value & 0b0000000011100000000000000000000000000000000000000000000000000000) >> 53;
    tiles[index].size    = (value & 0b0000000000011000000000000000000000000000000000000000000000000000) >> 51;
    tiles[index].line    = (value & 0b0000000000000011111111100000000000000000000000000000000000000000) >> 41;
    tiles[index].addr    = (value & 0b0000000000000000000000011111111100000000000000000000000000000000) >> 32;
    tiles[index].palette = (value & 0b0000000000000000000000000000000000000000111100000000000000000000) >> 20;
    tiles[index].ct      = (value & 0b0000000000000000000000000000000000000000000010000000000000000000) > 0;
    tiles[index].mt      = (value & 0b0000000000000000000000000000000000000000000001000000000000000000) > 0;
    tiles[index].mask_t  = (value & 0b0000000000000000000000000000000000000000000000111100000000000000) >> 14;
    tiles[index].shift_t = (value & 0b0000000000000000000000000000000000000000000000000011110000000000) >> 10;
    tiles[index].cs      = (value & 0b0000000000000000000000000000000000000000000000000000001000000000) > 0;
    tiles[index].ms      = (value & 0b0000000000000000000000000000000000000000000000000000000100000000) > 0;
    tiles[index].mask_s  = (value & 0b0000000000000000000000000000000000000000000000000000000011110000) >> 4;
    tiles[index].shift_s = (value & 0b0000000000000000000000000000000000000000000000000000000000001111);
    add_PC(8);
}

void cmd_LoadTile(uint64_t value)
{
    float SL      = get_ten_point_two((value & 0x00FFF00000000000) >> 44);
    float TL      = get_ten_point_two((value & 0x00000FFF00000000) >> 32);
    uint8_t index =                   (value & 0x0000000007000000) >> 24;
    float SH      = get_ten_point_two((value & 0x0000000000FFF000) >> 12);
    float TH      = get_ten_point_two((value & 0x0000000000000FFF));

    uint16_t tmem_addr = tiles[index].addr;
    uint32_t base_addr = curr_teximage.addr;

    uint8_t  bytes_per_pixel = 0;
    switch (curr_teximage.size)
    {
        case BPP_4:
            SL /= 2;
            TL /= 2;

            SH /= 2;
            TH /= 2;
            bytes_per_pixel = 1;
            break;
        case BPP_8:
            bytes_per_pixel = 1;
            break;
        case BPP_16:
            bytes_per_pixel = 2;
            break;
        case BPP_32:
            bytes_per_pixel = 4;
            break;
    }

    tiles[index].sh = SH;
    tiles[index].th = TH;

    uint32_t src = base_addr + (((uint32_t)SL + ((uint32_t)TL * (uint32_t)(SH + 1))) * bytes_per_pixel);
    size_t   len = ((uint32_t)(SH + 1) * (uint32_t)(TH + 1)) * bytes_per_pixel;

    void* real_src_addr = get_real_memory_loc(src);
    void* real_dst_addr = &RDP_TMEM[tmem_addr * 8];

    memcpy(real_dst_addr, real_src_addr, len);

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

void cmd_TextureRectangle(uint64_t value)
{
    texrect_t tex_rect;

    tex_rect.rect.XL = (value & 0x00FFF00000000000) >> 44;
    tex_rect.rect.YL = (value & 0x00000FFF00000000) >> 32;
    tex_rect.tile    = (value & 0x0000000007000000) >> 24;
    tex_rect.rect.XH = (value & 0x0000000000FFF000) >> 12;
    tex_rect.rect.YH = (value & 0x0000000000000FFF);
    
    uint64_t tex_coeff = get_coeff(8);

    tex_rect.S    = (tex_coeff & 0xFFFF000000000000) >> 48;
    tex_rect.T    = (tex_coeff & 0x0000FFFF00000000) >> 32;
    tex_rect.DsDx = (tex_coeff & 0x00000000FFFF0000) >> 16;
    tex_rect.DtDy = (tex_coeff & 0x000000000000FFFF);

    bool flip = (value & 0x0100000000000000) > 0;

    draw_tex_rect(&tex_rect, flip);

    add_PC(16);
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
    fill_color = value & 0xFFFFFFFF;
    add_PC(8);
}

void cmd_SetPrimColor(uint64_t value)
{
    curr_primcolor.prim_min_level  = (value & 0x00003F0000000000) >> 40;
    curr_primcolor.prim_level_frac = (value & 0x000000FF00000000) >> 32;
    curr_primcolor.color.red       = (value & 0x00000000FF000000) >> 24;
    curr_primcolor.color.green     = (value & 0x0000000000FF0000) >> 16;
    curr_primcolor.color.blue      = (value & 0x000000000000FF00) >> 8;
    curr_primcolor.color.alpha     = (value & 0x00000000000000FF);
    add_PC(8);
}

void cmd_SetBlendColor(uint64_t value)
{
    curr_blendcolor.red   = (value & 0x00000000FF000000) >> 24;
    curr_blendcolor.green = (value & 0x0000000000FF0000) >> 16;
    curr_blendcolor.blue  = (value & 0x000000000000FF00) >> 8;
    curr_blendcolor.alpha = (value & 0x00000000000000FF);
    add_PC(8);
}

void cmd_SetScissor(uint64_t value)
{
    scissor_border.border.XH = (value & 0x00FFF00000000000) >> 44;
    scissor_border.border.YH = (value & 0x00000FFF00000000) >> 32;
    scissor_border.border.XL = (value & 0x0000000000FFF000) >> 12;
    scissor_border.border.YL = (value & 0x0000000000000FFF);

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

void cmd_SyncTile(uint64_t value)
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

    edges.level = (value & 0x0038000000000000) >> 51;
    edges.tile  = (value & 0x0007000000000000) >> 48;

    edges.YL = (value & 0x00003FFF00000000) >> 32;
    edges.YM = (value & 0x000000003FFF0000) >> 16;
    edges.YH = (value & 0x0000000000003FFF);

    uint64_t edge_coeff1 = get_coeff(coeff_offset++ * 8);
    uint64_t edge_coeff2 = get_coeff(coeff_offset++ * 8);
    uint64_t edge_coeff3 = get_coeff(coeff_offset++ * 8);

    edges.XL         = (edge_coeff1 & 0xFFFF000000000000) >> 48;
    edges.XL_frac    = (edge_coeff1 & 0x0000FFFF00000000) >> 32;
    edges.DxLDy      = (edge_coeff1 & 0x00000000FFFF0000) >> 16;
    edges.DxLDy_frac = (edge_coeff1 & 0x000000000000FFFF);

    edges.XH         = (edge_coeff2 & 0xFFFF000000000000) >> 48;
    edges.XH_frac    = (edge_coeff2 & 0x0000FFFF00000000) >> 32;
    edges.DxHDy      = (edge_coeff2 & 0x00000000FFFF0000) >> 16;
    edges.DxHDy_frac = (edge_coeff2 & 0x000000000000FFFF);

    edges.XM         = (edge_coeff3 & 0xFFFF000000000000) >> 48;
    edges.XM_frac    = (edge_coeff3 & 0x0000FFFF00000000) >> 32;
    edges.DxMDy      = (edge_coeff3 & 0x00000000FFFF0000) >> 16;
    edges.DxMDy_frac = (edge_coeff3 & 0x000000000000FFFF);

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

        shade.red   = (shade_coeff1 & 0xFFFF000000000000) >> 48;
        shade.green = (shade_coeff1 & 0x0000FFFF00000000) >> 32;
        shade.blue  = (shade_coeff1 & 0x00000000FFFF0000) >> 16;
        shade.alpha = (shade_coeff1 & 0x000000000000FFFF);

        shade.DrDx = (shade_coeff2 & 0xFFFF000000000000) >> 48;
        shade.DgDx = (shade_coeff2 & 0x0000FFFF00000000) >> 32;
        shade.DbDx = (shade_coeff2 & 0x00000000FFFF0000) >> 16;
        shade.DaDx = (shade_coeff2 & 0x000000000000FFFF);

        shade.red_frac   = (shade_coeff3 & 0xFFFF000000000000) >> 48;
        shade.green_frac = (shade_coeff3 & 0x0000FFFF00000000) >> 32;
        shade.blue_frac  = (shade_coeff3 & 0x00000000FFFF0000) >> 16;
        shade.alpha_frac = (shade_coeff3 & 0x000000000000FFFF);

        shade.DrDx_frac = (shade_coeff4 & 0xFFFF000000000000) >> 48;
        shade.DgDx_frac = (shade_coeff4 & 0x0000FFFF00000000) >> 32;
        shade.DbDx_frac = (shade_coeff4 & 0x00000000FFFF0000) >> 16;
        shade.DaDx_frac = (shade_coeff4 & 0x000000000000FFFF);

        shade.DrDe = (shade_coeff5 & 0xFFFF000000000000) >> 48;
        shade.DgDe = (shade_coeff5 & 0x0000FFFF00000000) >> 32;
        shade.DbDe = (shade_coeff5 & 0x00000000FFFF0000) >> 16;
        shade.DaDe = (shade_coeff5 & 0x000000000000FFFF);

        shade.DrDy = (shade_coeff6 & 0xFFFF000000000000) >> 48;
        shade.DgDy = (shade_coeff6 & 0x0000FFFF00000000) >> 32;
        shade.DbDy = (shade_coeff6 & 0x00000000FFFF0000) >> 16;
        shade.DaDy = (shade_coeff6 & 0x000000000000FFFF);

        shade.DrDe_frac = (shade_coeff7 & 0xFFFF000000000000) >> 48;
        shade.DgDe_frac = (shade_coeff7 & 0x0000FFFF00000000) >> 32;
        shade.DbDe_frac = (shade_coeff7 & 0x00000000FFFF0000) >> 16;
        shade.DaDe_frac = (shade_coeff7 & 0x000000000000FFFF);

        shade.DrDy_frac = (shade_coeff8 & 0xFFFF000000000000) >> 48;
        shade.DgDy_frac = (shade_coeff8 & 0x0000FFFF00000000) >> 32;
        shade.DbDy_frac = (shade_coeff8 & 0x00000000FFFF0000) >> 16;
        shade.DaDy_frac = (shade_coeff8 & 0x000000000000FFFF);

        shade_point = &shade;
        add_PC(64);
    }

    texcoeff_t tex;
    texcoeff_t* tex_point = NULL;

    if ((flags & 0b0010) > 0) // Textured
    {
        uint64_t tex_coeff1 = get_coeff(coeff_offset++ * 8);
        uint64_t tex_coeff2 = get_coeff(coeff_offset++ * 8);
        uint64_t tex_coeff3 = get_coeff(coeff_offset++ * 8);
        uint64_t tex_coeff4 = get_coeff(coeff_offset++ * 8);
        uint64_t tex_coeff5 = get_coeff(coeff_offset++ * 8);
        uint64_t tex_coeff6 = get_coeff(coeff_offset++ * 8);
        uint64_t tex_coeff7 = get_coeff(coeff_offset++ * 8);
        uint64_t tex_coeff8 = get_coeff(coeff_offset++ * 8);

        tex.S = (tex_coeff1 & 0xFFFF000000000000) >> 48;
        tex.T = (tex_coeff1 & 0x0000FFFF00000000) >> 32;
        tex.W = (tex_coeff1 & 0x00000000FFFF0000) >> 16;

        tex.DsDx = (tex_coeff2 & 0xFFFF000000000000) >> 48;
        tex.DtDx = (tex_coeff2 & 0x0000FFFF00000000) >> 32;
        tex.DwDx = (tex_coeff2 & 0x00000000FFFF0000) >> 16;

        tex.S_frac = (tex_coeff3 & 0xFFFF000000000000) >> 48;
        tex.T_frac = (tex_coeff3 & 0x0000FFFF00000000) >> 32;
        tex.W_frac = (tex_coeff3 & 0x00000000FFFF0000) >> 16;

        tex.DsDx_frac = (tex_coeff4 & 0xFFFF000000000000) >> 48;
        tex.DtDx_frac = (tex_coeff4 & 0x0000FFFF00000000) >> 32;
        tex.DwDx_frac = (tex_coeff4 & 0x00000000FFFF0000) >> 16;

        tex.DsDe = (tex_coeff5 & 0xFFFF000000000000) >> 48;
        tex.DtDe = (tex_coeff5 & 0x0000FFFF00000000) >> 32;
        tex.DwDe = (tex_coeff5 & 0x00000000FFFF0000) >> 16;

        tex.DsDy = (tex_coeff6 & 0xFFFF000000000000) >> 48;
        tex.DtDy = (tex_coeff6 & 0x0000FFFF00000000) >> 32;
        tex.DwDy = (tex_coeff6 & 0x00000000FFFF0000) >> 16;

        tex.DsDe_frac = (tex_coeff7 & 0xFFFF000000000000) >> 48;
        tex.DtDe_frac = (tex_coeff7 & 0x0000FFFF00000000) >> 32;
        tex.DwDe_frac = (tex_coeff7 & 0x00000000FFFF0000) >> 16;

        tex.DsDy_frac = (tex_coeff8 & 0xFFFF000000000000) >> 48;
        tex.DtDy_frac = (tex_coeff8 & 0x0000FFFF00000000) >> 32;
        tex.DwDy_frac = (tex_coeff8 & 0x00000000FFFF0000) >> 16;

        tex_point = &tex;
        add_PC(64);
    }

    draw_triangle(&edges, shade_point, tex_point, NULL);

    add_PC(32);
}