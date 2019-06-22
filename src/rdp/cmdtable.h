#pragma once

#include <stdint.h>

typedef struct
{
    void (*interpret)(uint64_t value);
} cmd_t;

cmd_t CMDtable[0x3F+1];

void RDP_CMDtable_init(void);