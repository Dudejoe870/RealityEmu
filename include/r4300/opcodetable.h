#pragma once

#include <stdint.h>

#include "mips/cpu.h"

opcode_t CPU_opcode_table[0x3F+1]; // Indexed by the first six bits of the Instruction.

void CPU_opcode_table_init(void);