#pragma once

#include <stdint.h>
#include <stdbool.h>

bool     get_is_branching(void);
uint64_t get_all_cycles(void);

void step(void); // Step the CPU.

// These cover multiple Instructions.
void SPECIAL(uint32_t value);
void REGIMM (uint32_t value);
void COP0   (uint32_t value);
void COP1   (uint32_t value);

void ADDI  (uint32_t value);
void ADDIU (uint32_t value);
void DADDI (uint32_t value);
void DADDIU(uint32_t value);
void ANDI  (uint32_t value);
void ORI   (uint32_t value);
void XORI  (uint32_t value);

void LB (uint32_t value);
void LBU(uint32_t value);
void LD (uint32_t value);
void LDL(uint32_t value);
void LDR(uint32_t value);
void LH (uint32_t value);
void LHU(uint32_t value);
void LL (uint32_t value);
void LLD(uint32_t value);
void LUI(uint32_t value);
void LW (uint32_t value);
void LWL(uint32_t value);
void LWR(uint32_t value);
void LWU(uint32_t value);

void SB   (uint32_t value);
void SC   (uint32_t value);
void SCD  (uint32_t value);
void SD   (uint32_t value);
void SDL  (uint32_t value);
void SDR  (uint32_t value);
void SH   (uint32_t value);
void SLTI (uint32_t value);
void SLTIU(uint32_t value);
void SW   (uint32_t value);
void SWL  (uint32_t value);
void SWR  (uint32_t value);

void BEQ  (uint32_t value);
void BEQL (uint32_t value);
void BGTZ (uint32_t value);
void BGTZL(uint32_t value);
void BLEZ (uint32_t value);
void BLEZL(uint32_t value);
void BNE  (uint32_t value);
void BNEL (uint32_t value);

void J  (uint32_t value);
void JAL(uint32_t value);

void CACHE(uint32_t value);