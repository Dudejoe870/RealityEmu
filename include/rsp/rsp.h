#pragma once

#include "mips/cpu.h"

#define COP0_DMA_CACHE        0
#define COP0_DMA_DRAM         1
#define COP0_DMA_READ_LENGTH  2
#define COP0_DMA_WRITE_LENGTH 3
#define COP0_SP_STATUS        4
#define COP0_DMA_FULL         5
#define COP0_DMA_BUSY         6
#define COP0_SP_RESERVED      7
#define COP0_CMD_START        8
#define COP0_CMD_END          9
#define COP0_CMD_CURRENT      10
#define COP0_CMD_STATUS       11
#define COP0_CMD_CLOCK        12
#define COP0_CMD_BUSY         13
#define COP0_CMD_PIPE_BUSY    14
#define COP0_CMD_TMEM_BUSY    15

cpu_t rsp;

double RSP_mhz;

bool RSP_has_started;

void RSP_init(void);

void RSP_cleanup(void);

void RSP_start(void);