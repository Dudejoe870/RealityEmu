#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "mips/cpu.h"

// These cover multiple Instructions.
void SPECIAL(uint32_t value, cpu_t* cpu);
void REGIMM (uint32_t value, cpu_t* cpu);
void COP0   (uint32_t value, cpu_t* cpu);
void COP1   (uint32_t value, cpu_t* cpu);
void LDC1   (uint32_t value, cpu_t* cpu);
void LWC1   (uint32_t value, cpu_t* cpu);
void SDC1   (uint32_t value, cpu_t* cpu);
void SWC1   (uint32_t value, cpu_t* cpu);

void ADDI  (uint32_t value, cpu_t* cpu);
void ADDIU (uint32_t value, cpu_t* cpu);
void DADDI (uint32_t value, cpu_t* cpu);
void DADDIU(uint32_t value, cpu_t* cpu);
void ANDI  (uint32_t value, cpu_t* cpu);
void ORI   (uint32_t value, cpu_t* cpu);
void XORI  (uint32_t value, cpu_t* cpu);

void LB (uint32_t value, cpu_t* cpu);
void LBU(uint32_t value, cpu_t* cpu);
void LD (uint32_t value, cpu_t* cpu);
void LDL(uint32_t value, cpu_t* cpu);
void LDR(uint32_t value, cpu_t* cpu);
void LH (uint32_t value, cpu_t* cpu);
void LHU(uint32_t value, cpu_t* cpu);
void LL (uint32_t value, cpu_t* cpu);
void LLD(uint32_t value, cpu_t* cpu);
void LUI(uint32_t value, cpu_t* cpu);
void LW (uint32_t value, cpu_t* cpu);
void LWL(uint32_t value, cpu_t* cpu);
void LWR(uint32_t value, cpu_t* cpu);
void LWU(uint32_t value, cpu_t* cpu);

void SB   (uint32_t value, cpu_t* cpu);
void SC   (uint32_t value, cpu_t* cpu);
void SCD  (uint32_t value, cpu_t* cpu);
void SD   (uint32_t value, cpu_t* cpu);
void SDL  (uint32_t value, cpu_t* cpu);
void SDR  (uint32_t value, cpu_t* cpu);
void SH   (uint32_t value, cpu_t* cpu);
void SLTI (uint32_t value, cpu_t* cpu);
void SLTIU(uint32_t value, cpu_t* cpu);
void SW   (uint32_t value, cpu_t* cpu);
void SWL  (uint32_t value, cpu_t* cpu);
void SWR  (uint32_t value, cpu_t* cpu);

void BEQ  (uint32_t value, cpu_t* cpu);
void BEQL (uint32_t value, cpu_t* cpu);
void BGTZ (uint32_t value, cpu_t* cpu);
void BGTZL(uint32_t value, cpu_t* cpu);
void BLEZ (uint32_t value, cpu_t* cpu);
void BLEZL(uint32_t value, cpu_t* cpu);
void BNE  (uint32_t value, cpu_t* cpu);
void BNEL (uint32_t value, cpu_t* cpu);

void J  (uint32_t value, cpu_t* cpu);
void JAL(uint32_t value, cpu_t* cpu);

void CACHE(uint32_t value, cpu_t* cpu);