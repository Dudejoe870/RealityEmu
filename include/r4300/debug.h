#pragma once

#include <stdint.h>

#include "mips/cpu.h"

void dbg_memdump(uint32_t start, uint32_t end); // For debugging purposes only.
void dbg_printreg(cpu_t cpu); // For debugging purposes only.