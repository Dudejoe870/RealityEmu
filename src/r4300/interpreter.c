#include "interpreter.h"

#include <stdio.h>

#include "cpu.h"

void UnimplementedInstError(uint32_t Value)
{
    printf("ERROR: Unimplemented Instruction 0x%x\n", Value);
}

void SPECIAL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void REGIMM(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void ADDI(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void ADDIU(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void DADDI(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void DADDIU(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void ANDI(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void ORI(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void XORI(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LB (uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LBU(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LD (uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LDL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LDR(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LH (uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LHU(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LL (uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LLD(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LUI(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LW (uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LWL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LWR(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void LWU(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SB(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SC(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SCD(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SD(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SDL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SDR(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SH(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SLTI(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SLTIU(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SW(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SWL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void SWR(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void BEQ(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void BEQL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void BGTZ(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void BGTZL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void BLEZ(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void BLEZL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void BNE(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void BNEL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void J(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void JAL(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void CACHE(uint32_t Value)
{
    UnimplementedInstError(Value);
}

void COP0(uint32_t Value)
{
    UnimplementedInstError(Value);
}