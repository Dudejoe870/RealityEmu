#include "interpreter.h"

#include <stdio.h>
#include <stdbool.h>

#include "cpu.h"
#include "opcodetable.h"
#include "mem.h"

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

#define INST_OP(x)     (x & INST_OP_MSK)     >> INST_OP_SHIFT
#define INST_RS(x)     (x & INST_RS_MSK)     >> INST_RS_SHIFT
#define INST_RT(x)     (x & INST_RT_MSK)     >> INST_RT_SHIFT
#define INST_RD(x)     (x & INST_RD_MSK)     >> INST_RD_SHIFT
#define INST_SA(x)     (x & INST_SA_MSK)     >> INST_SA_SHIFT
#define INST_FUNCT(x)  (x & INST_FUNCT_MSK)  >> INST_FUNCT_SHIFT
#define INST_IMM(x)    (x & INST_IMM_MSK)    >> INST_IMM_SHIFT
#define INST_TARGET(x) (x & INST_TARGET_MSK) >> INST_TARGET_SHIFT
#define INST_COP(x)    (x & INST_COP_MSK)    >> INST_COP_SHIFT

bool IsRunning = true;

bool IsBranching = false;
int currTarget = 0;

static inline void UndefinedInstError(uint32_t Value)
{
    printf("ERROR: Unimplemented Instruction 0x%x\n", Value);
    IsRunning = false;
}

static inline void ADDReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((int)((int)ReadGPR(Reg1) + (int)ReadGPR(Reg2)), Dst);
}

static inline void ADDImm(uint8_t Reg, uint16_t Imm, uint8_t Dst)
{
    WriteGPR((int)((int)ReadGPR(Reg) + (short)Imm), Dst);
}

static inline void DIVReg(uint8_t Reg1, uint8_t Reg2)
{
    if ((uint32_t)ReadGPR(Reg2) == 0) return;
    WriteLO((int)ReadGPR(Reg1) / (int)ReadGPR(Reg2));
    WriteHI((int)ReadGPR(Reg1) % (int)ReadGPR(Reg2));
}

static inline void DIVUReg(uint8_t Reg1, uint8_t Reg2)
{
    if ((uint32_t)ReadGPR(Reg2) == 0) return;
    WriteLO((uint32_t)ReadGPR(Reg1) / (uint32_t)ReadGPR(Reg2));
    WriteHI((uint32_t)ReadGPR(Reg1) % (uint32_t)ReadGPR(Reg2));
}

static inline void DADDReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR(ReadGPR(Reg1) + ReadGPR(Reg2), Dst);
}

static inline void DADDImm(uint8_t Reg1, uint16_t Imm, uint8_t Dst)
{
    WriteGPR(ReadGPR(Reg1) + (short)Imm, Dst);
}

static inline void DDIVReg(uint8_t Reg1, uint8_t Reg2)
{
    if (ReadGPR(Reg2) == 0) return;
    WriteLO((long)ReadGPR(Reg1) / (long)ReadGPR(Reg2));
    WriteHI((long)ReadGPR(Reg1) % (long)ReadGPR(Reg2));
}

static inline void DDIVUReg(uint8_t Reg1, uint8_t Reg2)
{
    if (ReadGPR(Reg2) == 0) return;
    WriteLO(ReadGPR(Reg1) / ReadGPR(Reg2));
    WriteHI(ReadGPR(Reg1) % ReadGPR(Reg2));
}

static inline void DMULTReg(uint8_t Reg1, uint8_t Reg2)
{
    __int128 Res = (long)ReadGPR(Reg1) * (long)ReadGPR(Reg2);
    WriteLO(Res);
    WriteHI(Res >> 64);
}

static inline void DMULTUReg(uint8_t Reg1, uint8_t Reg2)
{
    __int128 Res = ReadGPR(Reg1) * ReadGPR(Reg2);
    WriteLO(Res);
    WriteHI(Res >> 64);
}

static inline void ANDReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((uint32_t)((uint32_t)ReadGPR(Reg1) & (uint32_t)ReadGPR(Reg2)), Dst);
}

static inline void ANDImm(uint8_t Reg, uint16_t Imm, uint8_t Dst)
{
    WriteGPR((uint32_t)ReadGPR(Reg) & Imm, Dst);
}

static inline void BRANCHCond(uint16_t Imm, bool Cond)
{
    IsBranching = Cond;
    if (Cond) currTarget = (int)(Imm << 2);
    // In the Main Loop we check after the next instruction has executed if we are branching
    // if so, we add the target (signed) to the PC.  Then reset the variables.
}

static inline void BRANCHCondLikely(uint16_t Imm, bool Cond)
{
    BRANCHCond(Imm, Cond);
    if (!Cond) AdvancePC();
}

static inline void Link(void)
{
    Regs.GPR[31].Value = Regs.PC.Value + 8; // Hyah!
}

void Step(void)
{
    if (IsRunning)
    {
        bool ShouldBranch = IsBranching;

        uint32_t inst = ReadUInt32(Regs.PC.Value);
        opcode_t op   = OpcodeTable[INST_OP(inst)];

        Regs.GPR[0].Value = 0;

        if (inst) op.Interpret(inst);
        else AdvancePC();

        if (ShouldBranch) // If we should branch (aka the last instruction was a branch instruction)
        {
            Regs.PC.Value -= 4; // De-advance the PC because interpreting the delay slot advances the PC
                                // but the branch address is computed with the address of the delay slot in mind.
            Regs.PC.Value += (int)currTarget; // Then add to the PC the target.
            IsBranching = false; // And reset the variables.
            currTarget = 0;
        }
    }
}

static inline void ADD(uint32_t Value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    ADDReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void ADDU(uint32_t Value)
{
    ADDReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void AND(uint32_t Value)
{
    ANDReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void DADD(uint32_t Value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    DADDReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void DADDU(uint32_t Value)
{
    DADDReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void DDIV(uint32_t Value)
{
    DDIVReg(INST_RS(Value), INST_RT(Value));
    AdvancePC();
}

static inline void DDIVU(uint32_t Value)
{
    DDIVUReg(INST_RS(Value), INST_RT(Value));
    AdvancePC();
}

static inline void DIV(uint32_t Value)
{
    DIVReg(INST_RS(Value), INST_RT(Value));
    AdvancePC();
}

static inline void DIVU(uint32_t Value)
{
    DIVUReg(INST_RS(Value), INST_RT(Value));
    AdvancePC();
}

static inline void DMULT(uint32_t Value)
{
    DMULTReg(INST_RS(Value), INST_RT(Value));
    AdvancePC();
}

static inline void DMULTU(uint32_t Value)
{
    DMULTUReg(INST_RS(Value), INST_RT(Value));
    AdvancePC();
}

void SPECIAL(uint32_t Value)
{
    if (INST_SA(Value) == 0)
    {
        switch (INST_FUNCT(Value))
        {
            case 0b100000: // ADD
                ADD(Value);
                return;
            case 0b100001: // ADDU
                ADDU(Value);
                return;
            case 0b100100: // AND
                AND(Value);
                return;
            case 0b101100: // DADD
                DADD(Value);
                return;
            case 0b101101: // DADDU
                DADDU(Value);
                return;
            case 0b011110: // DDIV
                DDIV(Value);
                return;
            case 0b011111: // DDIVU
                DDIVU(Value);
                return;
            case 0b011010: // DIV
                DIV(Value);
                return;
            case 0b011011: // DIVU
                DIVU(Value);
                return;
            case 0b011100: // DMULT
                DMULT(Value);
                return;
            case 0b011101: // DMULT
                DMULTU(Value);
                return;
        }
    }
    UndefinedInstError(Value);
}

static inline void BGEZ(uint32_t Value)
{
    BRANCHCond(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) >= 0);
    AdvancePC();
}

static inline void BGEZAL(uint32_t Value)
{
    Link();
    BRANCHCond(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) >= 0);
    AdvancePC();
}

static inline void BGEZALL(uint32_t Value)
{
    Link();
    BRANCHCondLikely(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) >= 0);
    AdvancePC();
}

static inline void BGEZL(uint32_t Value)
{
    BRANCHCondLikely(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) >= 0);
    AdvancePC();
}

static inline void BLTZ(uint32_t Value)
{
    BRANCHCond(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) < 0);
    AdvancePC();
}

static inline void BLTZAL(uint32_t Value)
{
    Link();
    BRANCHCond(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) < 0);
    AdvancePC();
}

static inline void BLTZALL(uint32_t Value)
{
    Link();
    BRANCHCondLikely(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) < 0);
    AdvancePC();
}

static inline void BLTZL(uint32_t Value)
{
    BRANCHCondLikely(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) < 0);
    AdvancePC();
}

void REGIMM(uint32_t Value)
{
    switch (INST_RT(Value))
    {
        case 0b00001: // BGEZ
            BGEZ(Value);
            return;
        case 0b10001: // BGEZAL
            BGEZAL(Value);
            return;
        case 0b10011: // BGEZALL
            BGEZALL(Value);
            return;
        case 0b00011: // BGEZL
            BGEZL(Value);
            return;
        case 0b00000: // BLTZ
            BLTZ(Value);
            return;
        case 0b10000: // BLTZAL
            BLTZAL(Value);
            return;
        case 0b10010: // BLTZALL
            BLTZALL(Value);
            return;
        case 0b00010: // BLTZL
            BLTZL(Value);
            return;
    }
    UndefinedInstError(Value);
}

void COPz(uint32_t Value)
{
    if (INST_COP(Value) == 0) // COP0
    {
    }
    else if (INST_COP(Value) == 1) // COP1
    {
    }
    UndefinedInstError(Value);
}

void ADDI(uint32_t Value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    ADDImm(INST_RS(Value), INST_IMM(Value), INST_RT(Value));
    AdvancePC();
}

void ADDIU(uint32_t Value)
{
    ADDImm(INST_RS(Value), INST_IMM(Value), INST_RT(Value));
    AdvancePC();
}

void DADDI(uint32_t Value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    DADDImm(INST_RS(Value), INST_IMM(Value), INST_RT(Value));
    AdvancePC();
}

void DADDIU(uint32_t Value)
{
    DADDImm(INST_RS(Value), INST_IMM(Value), INST_RT(Value));
    AdvancePC();
}

void ANDI(uint32_t Value)
{
    ANDImm(INST_RS(Value), INST_IMM(Value), INST_RT(Value));
    AdvancePC();
}

void ORI(uint32_t Value)
{
    UndefinedInstError(Value);
}

void XORI(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LB(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LBU(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LD(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LDL(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LDR(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LH(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LHU(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LL(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LLD(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LUI(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LW(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LWL(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LWR(uint32_t Value)
{
    UndefinedInstError(Value);
}

void LWU(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SB(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SC(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SCD(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SD(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SDL(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SDR(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SH(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SLTI(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SLTIU(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SW(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SWL(uint32_t Value)
{
    UndefinedInstError(Value);
}

void SWR(uint32_t Value)
{
    UndefinedInstError(Value);
}

void BEQ(uint32_t Value)
{
    BRANCHCond(INST_IMM(Value), ReadGPR(INST_RS(Value)) == ReadGPR(INST_RT(Value)));
    AdvancePC();
}

void BEQL(uint32_t Value)
{
    BRANCHCondLikely(INST_IMM(Value), ReadGPR(INST_RS(Value)) == ReadGPR(INST_RT(Value)));
    AdvancePC();
}

void BGTZ(uint32_t Value)
{
    BRANCHCond(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) > 0);
    AdvancePC();
}

void BGTZL(uint32_t Value)
{
    BRANCHCondLikely(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) > 0);
    AdvancePC();
}

void BLEZ(uint32_t Value)
{
    BRANCHCond(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) <= 0);
    AdvancePC();
}

void BLEZL(uint32_t Value)
{
    BRANCHCondLikely(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) <= 0);
    AdvancePC();
}

void BNE(uint32_t Value)
{
    BRANCHCond(INST_IMM(Value), ReadGPR(INST_RS(Value)) != ReadGPR(INST_RT(Value)));
    AdvancePC();
}

void BNEL(uint32_t Value)
{
    BRANCHCondLikely(INST_IMM(Value), ReadGPR(INST_RS(Value)) != ReadGPR(INST_RT(Value)));
    AdvancePC();
}

void J(uint32_t Value)
{
    UndefinedInstError(Value);
}

void JAL(uint32_t Value)
{
    UndefinedInstError(Value);
}

void CACHE(uint32_t Value)
{
    // Does nothing (we don't need to emulate this)
}