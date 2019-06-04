#pragma once

#include <stdint.h>

typedef struct
{
    void (*Interpret)(uint64_t Value);
} cmd_t;

cmd_t CMDTable[0x3F+1];

void RDP_CMDTableInit(void);