#pragma once

#include <stdint.h>
#include "r4300/r4300.h"

#define INST_OP_MSK     0b11111100000000000000000000000000
#define INST_RS_MSK     0b00000011111000000000000000000000
#define INST_RT_MSK     0b00000000000111110000000000000000
#define INST_RD_MSK     0b00000000000000001111100000000000
#define INST_SA_MSK     0b00000000000000000000011111000000
#define INST_FUNCT_MSK  0b00000000000000000000000000111111
#define INST_IMM_MSK    0b00000000000000001111111111111111
#define INST_TARGET_MSK 0b00000011111111111111111111111111
#define INST_COP_MSK    0b00001100000000000000000000000000

#define INST_OP_SHIFT     26
#define INST_RS_SHIFT     21
#define INST_RT_SHIFT     16
#define INST_RD_SHIFT     11
#define INST_SA_SHIFT     6
#define INST_FUNCT_SHIFT  0
#define INST_IMM_SHIFT    0
#define INST_TARGET_SHIFT 0
#define INST_COP_SHIFT    24

#define INST_OP(x)     ((x & INST_OP_MSK)     >> INST_OP_SHIFT)
#define INST_RS(x)     ((x & INST_RS_MSK)     >> INST_RS_SHIFT)
#define INST_RT(x)     ((x & INST_RT_MSK)     >> INST_RT_SHIFT)
#define INST_RD(x)     ((x & INST_RD_MSK)     >> INST_RD_SHIFT)
#define INST_SA(x)     ((x & INST_SA_MSK)     >> INST_SA_SHIFT)
#define INST_FUNCT(x)  ((x & INST_FUNCT_MSK)  >> INST_FUNCT_SHIFT)
#define INST_IMM(x)    ((x & INST_IMM_MSK)    >> INST_IMM_SHIFT)
#define INST_TARGET(x) ((x & INST_TARGET_MSK) >> INST_TARGET_SHIFT)
#define INST_COP(x)    ((x & INST_COP_MSK)    >> INST_COP_SHIFT)

#define INST_FMT(x) INST_RS(x)
#define INST_FT(x)  INST_RT(x)
#define INST_FS(x)  INST_RD(x)
#define INST_FD(x)  INST_SA(x)

__attribute__((__always_inline__)) static inline void MIPS_undefined_inst_error(uint32_t value, cpu_t* cpu)
{
    fprintf(stderr, "%s: ERROR: Unimplemented Instruction 0x%x!  PC: 0x%x\n", cpu->rsp ? "RSP" : "CPU", value, (uint32_t)cpu->regs.PC.value);
    is_running = false;
}