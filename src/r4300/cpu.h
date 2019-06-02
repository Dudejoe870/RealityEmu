#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    uint64_t Value;

    void (*WriteCallback)(void);
    void (*ReadCallback)(void);
} reg_t;

#define COP0_Index       0
#define COP0_Random      1
#define COP0_EntryLo0    2
#define COP0_EntryLo1    3
#define COP0_Context     4
#define COP0_PageMask    5
#define COP0_Wired       6
#define COP0_BadVAddr    8
#define COP0_Count       9
#define COP0_EntryHi     10
#define COP0_Compare     11
#define COP0_Status      12
#define COP0_Cause       13
#define COP0_EPC         14
#define COP0_PRId        15
#define COP0_Config      16
#define COP0_LLAddr      17
#define COP0_WatchLo     18
#define COP0_WatchHi     19
#define COP0_XContext    20
#define COP0_ParityError 26
#define COP0_CacheError  27
#define COP0_TagLo       28
#define COP0_TagHi       29
#define COP0_ErrorEPC    30

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

regs_t Regs;

bool IsRunning;
bool InvokeVIInterrupt;

void CPUInit(void* ROM, size_t ROMSize);

void CPUDeInit(void);

static inline void WriteGPR(uint64_t Value, uint8_t Index)
{
    Regs.GPR[Index].Value = Value;
    if (Regs.GPR[Index].WriteCallback) Regs.GPR[Index].WriteCallback();
}

static inline uint64_t ReadGPR(uint8_t Index)
{
    if (Regs.GPR[Index].ReadCallback) Regs.GPR[Index].ReadCallback();
    return Regs.GPR[Index].Value;
}

static inline void WriteFPR(uint64_t Value, uint8_t Index)
{
    Regs.FPR[Index].Value = Value;
    if (Regs.FPR[Index].WriteCallback) Regs.FPR[Index].WriteCallback();
}

static inline uint64_t ReadFPR(uint8_t Index)
{
    if (Regs.FPR[Index].ReadCallback) Regs.FPR[Index].ReadCallback();
    return Regs.FPR[Index].Value;
}

static inline void WriteCOP0(uint64_t Value, uint8_t Index)
{
    Regs.COP0[Index].Value = Value;
    if (Regs.COP0[Index].WriteCallback) Regs.COP0[Index].WriteCallback();
}

static inline uint64_t ReadCOP0(uint8_t Index)
{
    if (Regs.COP0[Index].ReadCallback) Regs.COP0[Index].ReadCallback();
    return Regs.COP0[Index].Value;
}

static inline void WritePC(uint32_t Value)
{
    Regs.PC.Value = (uint64_t)Value;
    if (Regs.PC.WriteCallback) Regs.PC.WriteCallback();
}

static inline uint32_t ReadPC(void)
{
    if (Regs.PC.ReadCallback) Regs.PC.ReadCallback();
    return Regs.PC.Value;
}

static inline void AdvancePC(void)
{
    WritePC(ReadPC() + 4);
}

static inline void WriteHI(uint64_t Value)
{
    Regs.HI.Value = Value;
    if (Regs.HI.WriteCallback) Regs.HI.WriteCallback();
}

static inline uint64_t ReadHI(void)
{
    if (Regs.HI.ReadCallback) Regs.HI.ReadCallback();
    return Regs.HI.Value;
}

static inline void WriteLO(uint64_t Value)
{
    Regs.LO.Value = Value;
    if (Regs.LO.WriteCallback) Regs.LO.WriteCallback();
}

static inline uint64_t ReadLO(void)
{
    if (Regs.LO.ReadCallback) Regs.LO.ReadCallback();
    return Regs.LO.Value;
}