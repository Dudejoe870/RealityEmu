#pragma once

#include <stdint.h>
#include "r4300/r4300.h"

typedef struct
{
    void (*interpret)(uint32_t value, cpu_t* cpu);
} opcode_t;

opcode_t opcode_table[0x3F+1]; // Indexed by the first six bits of the Instruction.

void opcode_table_init(void);