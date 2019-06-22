#pragma once

#include <stdint.h>

void RDP_step(void);

void cmd_SetColorImage(uint64_t value);
void cmd_FillRectangle(uint64_t value);
void cmd_SetOtherModes(uint64_t value);
void cmd_SetFillColor (uint64_t value);
void cmd_SetScissor   (uint64_t value);
void cmd_SyncFull     (uint64_t value);
void cmd_SyncPipe     (uint64_t value);

void cmd_Triangle(uint64_t value);