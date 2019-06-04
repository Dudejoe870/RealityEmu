#pragma once

#include <stdint.h>

#define MI_INTR_VI 3
#define MI_INTR_DP 5

void InvokeMIInterrupt(uint8_t Interrupt);