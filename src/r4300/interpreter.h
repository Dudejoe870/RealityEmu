#pragma once

#include <stdint.h>

// These cover multiple Instructions.
void SPECIAL(uint32_t Value);
void REGIMM (uint32_t Value);

void ADDI  (uint32_t Value);
void ADDIU (uint32_t Value);
void DADDI (uint32_t Value);
void DADDIU(uint32_t Value);
void ANDI  (uint32_t Value);
void ORI   (uint32_t Value);
void XORI  (uint32_t Value);

void LB (uint32_t Value);
void LBU(uint32_t Value);
void LD (uint32_t Value);
void LDL(uint32_t Value);
void LDR(uint32_t Value);
void LH (uint32_t Value);
void LHU(uint32_t Value);
void LL (uint32_t Value);
void LLD(uint32_t Value);
void LUI(uint32_t Value);
void LW (uint32_t Value);
void LWL(uint32_t Value);
void LWR(uint32_t Value);
void LWU(uint32_t Value);

void SB   (uint32_t Value);
void SC   (uint32_t Value);
void SCD  (uint32_t Value);
void SD   (uint32_t Value);
void SDL  (uint32_t Value);
void SDR  (uint32_t Value);
void SH   (uint32_t Value);
void SLTI (uint32_t Value);
void SLTIU(uint32_t Value);
void SW   (uint32_t Value);
void SWL  (uint32_t Value);
void SWR  (uint32_t Value);

void BEQ  (uint32_t Value);
void BEQL (uint32_t Value);
void BGTZ (uint32_t Value);
void BGTZL(uint32_t Value);
void BLEZ (uint32_t Value);
void BLEZL(uint32_t Value);
void BNE  (uint32_t Value);
void BNEL (uint32_t Value);

void J  (uint32_t Value);
void JAL(uint32_t Value);

void CACHE(uint32_t Value);

void COP0(uint32_t Value);