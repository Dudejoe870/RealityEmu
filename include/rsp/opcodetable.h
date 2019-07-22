#pragma once

#include <stdint.h>

#include "mips/cpu.h"

opcode_t RSP_opcode_table[0x3F+1]; // Indexed by the first six bits of the Instruction.

void RSP_opcode_table_init(void);