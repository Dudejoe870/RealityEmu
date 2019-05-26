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

typedef struct
{
    reg_t GPR[32];
    reg_t FPR[32]; // 0: FCR0 (32-bit), 31: FCR31 (32-bit)
    reg_t PC;
    reg_t HI;
    reg_t LO;
    bool LLbit;
} regs_t;

regs_t Regs;

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