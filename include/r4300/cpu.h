#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct
{
    uint64_t value;

    void (*write_callback)(void);
    void (*read_callback)(void);
} reg_t;

#define COP0_INDEX       0
#define COP0_RANDOM      1
#define COP0_ENTRYLO0    2
#define COP0_ENTRYLO1    3
#define COP0_CONTEXT     4
#define COP0_PAGEMASK    5
#define COP0_WIRED       6
#define COP0_BADVADDR    8
#define COP0_COUNT       9
#define COP0_ENTRYHI     10
#define COP0_COMPARE     11
#define COP0_STATUS      12
#define COP0_CAUSE       13
#define COP0_EPC         14
#define COP0_PRID        15
#define COP0_CONFIG      16
#define COP0_LLADDR      17
#define COP0_WATCHLO     18
#define COP0_WATCHHI     19
#define COP0_XCONTEXT    20
#define COP0_PARITYERROR 26
#define COP0_CACHEERROR  27
#define COP0_TAGLO       28
#define COP0_TAGHI       29
#define COP0_ERROREPC    30

typedef struct
{
    reg_t GPR[32];
    reg_t FPR[32]; // 0: FCR0 (32-bit), 31: FCR31 (32-bit)
    reg_t COP0[32];
    reg_t PC;
    reg_t HI;
    reg_t LO;
    bool  LLbit;
    bool  COC1;
} regs_t;

typedef struct
{
    regs_t regs;
    bool is_branching;

    uint32_t curr_target;

    uint8_t  curr_inst_cycles;
    uint64_t cycles;
    uint64_t all_cycles;
} cpu_t;

cpu_t r4300;

bool is_running;

double CPU_mhz;

void CPU_init(void* ROM, size_t ROM_size);

void CPU_cleanup(void);

__attribute__((__always_inline__)) static inline void write_GPR(uint64_t value, uint8_t index, cpu_t* cpu)
{
    cpu->regs.GPR[index].value = value;
    if (cpu->regs.GPR[index].write_callback) cpu->regs.GPR[index].write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_GPR(uint8_t index, cpu_t* cpu)
{
    if (cpu->regs.GPR[index].read_callback) cpu->regs.GPR[index].read_callback();
    return cpu->regs.GPR[index].value;
}

__attribute__((__always_inline__)) static inline void write_FPR(uint64_t value, uint8_t index, cpu_t* cpu)
{
    bool FR = (cpu->regs.COP0[COP0_STATUS].value & 0x04000000) > 0;

    if (index & 1 && !FR) return;
    
    cpu->regs.FPR[index].value = value;
    if (cpu->regs.FPR[index].write_callback) cpu->regs.FPR[index].write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_FPR(uint8_t index, cpu_t* cpu)
{
    bool FR = (cpu->regs.COP0[COP0_STATUS].value & 0x04000000) > 0;

    if (index & 1 && !FR) return 0;

    if (cpu->regs.FPR[index].read_callback) cpu->regs.FPR[index].read_callback();
    return cpu->regs.FPR[index].value;
}

__attribute__((__always_inline__)) static inline void write_COP0(uint64_t value, uint8_t index, cpu_t* cpu)
{
    cpu->regs.COP0[index].value = value;
    if (cpu->regs.COP0[index].write_callback) cpu->regs.COP0[index].write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_COP0(uint8_t index, cpu_t* cpu)
{
    if (cpu->regs.COP0[index].read_callback) cpu->regs.COP0[index].read_callback();
    return cpu->regs.COP0[index].value;
}

__attribute__((__always_inline__)) static inline void write_PC(uint32_t value, cpu_t* cpu)
{
    cpu->regs.PC.value = (uint64_t)value;
    if (cpu->regs.PC.write_callback) cpu->regs.PC.write_callback();
}

__attribute__((__always_inline__)) static inline uint32_t read_PC(cpu_t* cpu)
{
    if (cpu->regs.PC.read_callback) cpu->regs.PC.read_callback();
    return cpu->regs.PC.value;
}

__attribute__((__always_inline__)) static inline void advance_PC(cpu_t* cpu)
{
    write_PC(read_PC(cpu) + 4, cpu);
}

__attribute__((__always_inline__)) static inline void write_HI(uint64_t value, cpu_t* cpu)
{
    cpu->regs.HI.value = value;
    if (cpu->regs.HI.write_callback) cpu->regs.HI.write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_HI(cpu_t* cpu)
{
    if (cpu->regs.HI.read_callback) cpu->regs.HI.read_callback();
    return cpu->regs.HI.value;
}

__attribute__((__always_inline__)) static inline void write_LO(uint64_t value, cpu_t* cpu)
{
    cpu->regs.LO.value = value;
    if (cpu->regs.LO.write_callback) cpu->regs.LO.write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_LO(cpu_t* cpu)
{
    if (cpu->regs.LO.read_callback) cpu->regs.LO.read_callback();
    return cpu->regs.LO.value;
}