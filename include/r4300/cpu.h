#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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
    bool LLbit;
} regs_t;

regs_t regs;

bool is_running;

double CPU_mhz;

void CPU_init(void* ROM, size_t ROM_size);

void CPU_cleanup(void);

__attribute__((__always_inline__)) static inline void write_GPR(uint64_t value, uint8_t index)
{
    regs.GPR[index].value = value;
    if (regs.GPR[index].write_callback) regs.GPR[index].write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_GPR(uint8_t index)
{
    if (regs.GPR[index].read_callback) regs.GPR[index].read_callback();
    return regs.GPR[index].value;
}

__attribute__((__always_inline__)) static inline void write_FPR(uint64_t value, uint8_t index)
{
    bool FR    = (regs.COP0[COP0_STATUS].value & 0x02000000) > 0;
    bool is_CR = (index == 0 || index == 31);

    if (index & 1 && !FR) return;
    
    regs.FPR[index].value = (FR || is_CR) ? (uint32_t)value : value;
    if (regs.FPR[index].write_callback) regs.FPR[index].write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_FPR(uint8_t index)
{
    bool FR    = (regs.COP0[COP0_STATUS].value & 0x02000000) > 0;
    bool is_CR = (index == 0 || index == 31);

    if (index & 1 && !FR) return 0;

    if (regs.FPR[index].read_callback) regs.FPR[index].read_callback();
    return (FR || is_CR) ? (uint32_t)regs.FPR[index].value : regs.FPR[index].value;
}

__attribute__((__always_inline__)) static inline void write_COP0(uint64_t value, uint8_t index)
{
    regs.COP0[index].value = value;
    if (regs.COP0[index].write_callback) regs.COP0[index].write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_COP0(uint8_t index)
{
    if (regs.COP0[index].read_callback) regs.COP0[index].read_callback();
    return regs.COP0[index].value;
}

__attribute__((__always_inline__)) static inline void write_PC(uint32_t value)
{
    regs.PC.value = (uint64_t)value;
    if (regs.PC.write_callback) regs.PC.write_callback();
}

__attribute__((__always_inline__)) static inline uint32_t read_PC(void)
{
    if (regs.PC.read_callback) regs.PC.read_callback();
    return regs.PC.value;
}

__attribute__((__always_inline__)) static inline void advance_PC(void)
{
    write_PC(read_PC() + 4);
}

__attribute__((__always_inline__)) static inline void write_HI(uint64_t value)
{
    regs.HI.value = value;
    if (regs.HI.write_callback) regs.HI.write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_HI(void)
{
    if (regs.HI.read_callback) regs.HI.read_callback();
    return regs.HI.value;
}

__attribute__((__always_inline__)) static inline void write_LO(uint64_t value)
{
    regs.LO.value = value;
    if (regs.LO.write_callback) regs.LO.write_callback();
}

__attribute__((__always_inline__)) static inline uint64_t read_LO(void)
{
    if (regs.LO.read_callback) regs.LO.read_callback();
    return regs.LO.value;
}