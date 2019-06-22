#pragma once

#include <stdint.h>

typedef struct
{
    void (*interpret)(uint32_t value);
} opcode_t;

opcode_t opcode_table[0x3F+1]; // Indexed by the first six bits of the Instruction.

void opcode_table_init(void);