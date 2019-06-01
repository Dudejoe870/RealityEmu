#pragma once

#include <stdint.h>
#include <stdbool.h>

void InvokeTLBMiss(uint32_t Addr, bool Store);
void InvokeBreak  (void);
void InvokeTrap   (void);

void PollInt(void);