#pragma once

#include <stdint.h>

void RDP_step(void);

void cmd_SetColorImage   (uint64_t value);
void cmd_SetTextureImage (uint64_t value);
void cmd_SetTile         (uint64_t value);
void cmd_LoadTile        (uint64_t value);
void cmd_FillRectangle   (uint64_t value);
void cmd_TextureRectangle(uint64_t value);
void cmd_SetCombineMode  (uint64_t value);
void cmd_SetOtherModes   (uint64_t value);
void cmd_SetPrimColor    (uint64_t value);
void cmd_SetBlendColor   (uint64_t value);
void cmd_SetFillColor    (uint64_t value);
void cmd_SetScissor      (uint64_t value);
void cmd_SyncFull        (uint64_t value);
void cmd_SyncPipe        (uint64_t value);
void cmd_SyncTile        (uint64_t value);

void cmd_Triangle(uint64_t value);