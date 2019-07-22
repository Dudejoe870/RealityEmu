#pragma once

#include <stdint.h>

#define MI_INTR_SP 0
#define MI_INTR_SI 1
#define MI_INTR_AI 2
#define MI_INTR_VI 3
#define MI_INTR_PI 4
#define MI_INTR_DP 5

void invoke_mi_interrupt(uint8_t interrupt);