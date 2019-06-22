#pragma once

#include <stdint.h>

#define MI_INTR_VI 3
#define MI_INTR_DP 5

void invoke_mi_interrupt(uint8_t interrupt);