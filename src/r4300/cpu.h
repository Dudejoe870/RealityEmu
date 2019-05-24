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

void WriteGPR(uint64_t Value, uint8_t Index);
uint64_t ReadGPR(uint8_t Index);

void WriteFPR(uint64_t Value, uint8_t Index);
uint64_t ReadFPR(uint8_t Index);

void WritePC(uint32_t Value);
uint32_t ReadPC(void);
void AdvancePC(void);

void WriteHI(uint64_t Value);
uint64_t ReadHI(void);

void WriteLO(uint64_t Value);
uint64_t ReadLO(void);