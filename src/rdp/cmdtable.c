#include "cmdtable.h"

#include "interpreter.h"

void RDP_CMDTableInit(void)
{
    CMDTable[0x3f].Interpret = &SetColorImage;
    CMDTable[0x36].Interpret = &FillRectangle;
    CMDTable[0x2f].Interpret = &SetOtherModes;
    CMDTable[0x37].Interpret = &SetFillColor;
    CMDTable[0x2d].Interpret = &SetScissor;
    CMDTable[0x29].Interpret = &SyncFull;
    CMDTable[0x27].Interpret = &SyncPipe;

    // All of these are Triangle operations with different coefficients
    // Note: The reason why these all point to the same function is because each bit of the opcode
    // is just a flag turning on and off the different coefficients.
    CMDTable[0x08].Interpret = &Triangle; // Non-Shaded Triangle
    CMDTable[0x0c].Interpret = &Triangle; // Shaded Triangle
    CMDTable[0x0a].Interpret = &Triangle; // Textured Triangle
    CMDTable[0x0e].Interpret = &Triangle; // Shaded, Textured Triangle
    CMDTable[0x09].Interpret = &Triangle; // Non-Shaded, ZBuffered Triangle
    CMDTable[0x0d].Interpret = &Triangle; // Shaded, ZBuffered Triangle
    CMDTable[0x0b].Interpret = &Triangle; // Textured, ZBuffered Triangle
    CMDTable[0x0f].Interpret = &Triangle; // Shaded, Textured, ZBuffered Triangle
}