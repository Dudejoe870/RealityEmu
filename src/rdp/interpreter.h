#pragma once

#include <stdint.h>

void RDPStep(void);

void SetColorImage(uint64_t Value);
void FillRectangle(uint64_t Value);
void SetOtherModes(uint64_t Value);
void SetFillColor (uint64_t Value);
void SetScissor   (uint64_t Value);
void SyncFull     (uint64_t Value);
void SyncPipe     (uint64_t Value);

void Triangle(uint64_t Value);