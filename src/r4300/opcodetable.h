#pragma once

#include <stdint.h>

typedef struct
{
    void (*Interpret)(uint32_t Value);
} opcode_t;

opcode_t OpcodeTable[0x3F]; // Indexed by the first six bits of the Instruction.

void OpcodeTableInit(void);