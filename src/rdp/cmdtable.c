#include "common.h"

void RDP_CMDtable_init(void)
{
    CMDtable[0x3f].interpret = &cmd_SetColorImage;
    CMDtable[0x3d].interpret = &cmd_SetTextureImage;
    CMDtable[0x35].interpret = &cmd_SetTile;
    CMDtable[0x34].interpret = &cmd_LoadTile;
    CMDtable[0x36].interpret = &cmd_FillRectangle;
    CMDtable[0x24].interpret = &cmd_TextureRectangle;
    CMDtable[0x25].interpret = &cmd_TextureRectangle;
    CMDtable[0x3c].interpret = &cmd_SetCombineMode;
    CMDtable[0x2f].interpret = &cmd_SetOtherModes;
    CMDtable[0x3a].interpret = &cmd_SetPrimColor;
    CMDtable[0x39].interpret = &cmd_SetBlendColor;
    CMDtable[0x37].interpret = &cmd_SetFillColor;
    CMDtable[0x2d].interpret = &cmd_SetScissor;
    CMDtable[0x29].interpret = &cmd_SyncFull;
    CMDtable[0x27].interpret = &cmd_SyncPipe;
    CMDtable[0x28].interpret = &cmd_SyncTile;

    // All of these are Triangle operations with different coefficients
    // Note: The reason why these all point to the same function is because each bit of the opcode
    // is just a flag turning on and off the different coefficients.
    CMDtable[0x08].interpret = &cmd_Triangle; // Non-Shaded Triangle
    CMDtable[0x0c].interpret = &cmd_Triangle; // Shaded Triangle
    CMDtable[0x0a].interpret = &cmd_Triangle; // Textured Triangle
    CMDtable[0x0e].interpret = &cmd_Triangle; // Shaded, Textured Triangle
    CMDtable[0x09].interpret = &cmd_Triangle; // Non-Shaded, ZBuffered Triangle
    CMDtable[0x0d].interpret = &cmd_Triangle; // Shaded, ZBuffered Triangle
    CMDtable[0x0b].interpret = &cmd_Triangle; // Textured, ZBuffered Triangle
    CMDtable[0x0f].interpret = &cmd_Triangle; // Shaded, Textured, ZBuffered Triangle
}