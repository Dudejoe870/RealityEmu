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

#define INST_OP(x)     ((x & INST_OP_MSK)     >> INST_OP_SHIFT)
#define INST_RS(x)     ((x & INST_RS_MSK)     >> INST_RS_SHIFT)
#define INST_RT(x)     ((x & INST_RT_MSK)     >> INST_RT_SHIFT)
#define INST_RD(x)     ((x & INST_RD_MSK)     >> INST_RD_SHIFT)
#define INST_SA(x)     ((x & INST_SA_MSK)     >> INST_SA_SHIFT)
#define INST_FUNCT(x)  ((x & INST_FUNCT_MSK)  >> INST_FUNCT_SHIFT)
#define INST_IMM(x)    ((x & INST_IMM_MSK)    >> INST_IMM_SHIFT)
#define INST_TARGET(x) ((x & INST_TARGET_MSK) >> INST_TARGET_SHIFT)
#define INST_COP(x)    ((x & INST_COP_MSK)    >> INST_COP_SHIFT)

bool IsBranching = false;
uint32_t currTarget = 0;

static inline void UndefinedInstError(uint32_t Value)
{
    fprintf(stderr, "ERROR: Unimplemented Instruction 0x%x!  PC: 0x%x\n", Value, (uint32_t)Regs.PC.Value);
    IsRunning = false;
}

static inline void ADDReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((long)((uint32_t)ReadGPR(Reg1) + (uint32_t)ReadGPR(Reg2)), Dst);
}

static inline void ADDImm(uint8_t Reg, uint16_t Imm, uint8_t Dst)
{
    WriteGPR((long)((uint32_t)ReadGPR(Reg) + (short)Imm), Dst);
}

static inline void SUBReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((long)((uint32_t)ReadGPR(Reg1) - (uint32_t)ReadGPR(Reg2)), Dst);
}

static inline void DIVReg(uint8_t Reg1, uint8_t Reg2)
{
    if ((uint32_t)ReadGPR(Reg2) == 0) return;
    WriteLO((long)((int)ReadGPR(Reg1) / (int)ReadGPR(Reg2)));
    WriteHI((long)((int)ReadGPR(Reg1) % (int)ReadGPR(Reg2)));
}

static inline void DIVUReg(uint8_t Reg1, uint8_t Reg2)
{
    if ((uint32_t)ReadGPR(Reg2) == 0) return;
    WriteLO((uint32_t)ReadGPR(Reg1) / (uint32_t)ReadGPR(Reg2));
    WriteHI((uint32_t)ReadGPR(Reg1) % (uint32_t)ReadGPR(Reg2));
}

static inline void MULTReg(uint8_t Reg1, uint8_t Reg2)
{
    long Res = (long)((int)ReadGPR(Reg1) * (int)ReadGPR(Reg2));
    WriteLO(Res & 0xFFFFFFFF);
    WriteHI(Res >> 32);
}

static inline void MULTUReg(uint8_t Reg1, uint8_t Reg2)
{
    long Res = (long)((uint32_t)ReadGPR(Reg1) * (uint32_t)ReadGPR(Reg2));
    WriteLO(Res & 0xFFFFFFFF);
    WriteHI(Res >> 32);
}

static inline void SLLImm(uint8_t Reg, uint8_t Dst, uint8_t sa)
{
    WriteGPR(ReadGPR(Reg) << sa, Dst);
}

static inline void SLLReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((uint32_t)ReadGPR(Reg1) << (ReadGPR(Reg2) & 0x1F), Dst);
}

static inline void SRAImm(uint8_t Reg, uint8_t Dst, uint8_t sa)
{
    WriteGPR((long)((int)ReadGPR(Reg) >> sa), Dst);
}

static inline void SRAReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((long)((int)ReadGPR(Reg1) >> (ReadGPR(Reg2) & 0x1F)), Dst);
}

static inline void SRLImm(uint8_t Reg, uint8_t Dst, uint8_t sa)
{
    WriteGPR((uint32_t)ReadGPR(Reg) >> sa, Dst);
}

static inline void SRLReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR(ReadGPR(Reg1) >> (ReadGPR(Reg2) & 0x1F), Dst);
}

static inline void DADDReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR(ReadGPR(Reg1) + ReadGPR(Reg2), Dst);
}

static inline void DADDImm(uint8_t Reg1, uint16_t Imm, uint8_t Dst)
{
    WriteGPR(ReadGPR(Reg1) + (short)Imm, Dst);
}

static inline void DSUBReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR(ReadGPR(Reg1) - ReadGPR(Reg2), Dst);
}

static inline void DDIVReg(uint8_t Reg1, uint8_t Reg2)
{
    if (ReadGPR(Reg2) == 0) return;
    WriteLO((long)((long)ReadGPR(Reg1) / (long)ReadGPR(Reg2)));
    WriteHI((long)((long)ReadGPR(Reg1) % (long)ReadGPR(Reg2)));
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

static inline void DSLLImm(uint8_t Reg, uint8_t Dst, uint8_t sa)
{
    WriteGPR(ReadGPR(Reg) << sa, Dst);
}

static inline void DSLLReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR(ReadGPR(Reg1) << (ReadGPR(Reg2) & 0x1F), Dst);
}

static inline void DSRAImm(uint8_t Reg, uint8_t Dst, uint8_t sa)
{
    WriteGPR((long)ReadGPR(Reg) >> sa, Dst);
}

static inline void DSRAReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((long)ReadGPR(Reg1) >> (ReadGPR(Reg2) & 0x1F), Dst);
}

static inline void DSRLImm(uint8_t Reg, uint8_t Dst, uint8_t sa)
{
    WriteGPR(ReadGPR(Reg) >> sa, Dst);
}

static inline void DSRLReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR(ReadGPR(Reg1) >> (ReadGPR(Reg2) & 0x1F), Dst);
}

static inline void ANDReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((long)((uint32_t)ReadGPR(Reg1) & (uint32_t)ReadGPR(Reg2)), Dst);
}

static inline void ANDImm(uint8_t Reg, uint16_t Imm, uint8_t Dst)
{
    WriteGPR((long)((uint32_t)ReadGPR(Reg) & Imm), Dst);
}

static inline void ORReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((long)((uint32_t)ReadGPR(Reg1) | (uint32_t)ReadGPR(Reg2)), Dst);
}

static inline void ORImm(uint8_t Reg, uint16_t Imm, uint8_t Dst)
{
    WriteGPR((long)((uint32_t)ReadGPR(Reg) | Imm), Dst);
}

static inline void XORReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((long)((uint32_t)ReadGPR(Reg1) ^ (uint32_t)ReadGPR(Reg2)), Dst);
}

static inline void XORImm(uint8_t Reg, uint16_t Imm, uint8_t Dst)
{
    WriteGPR((long)((uint32_t)ReadGPR(Reg) ^ Imm), Dst);
}

static inline void NORReg(uint8_t Reg1, uint8_t Reg2, uint8_t Dst)
{
    WriteGPR((long)(~((uint32_t)ReadGPR(Reg1) | (uint32_t)ReadGPR(Reg2))), Dst);
}

static inline void SETCond(uint8_t Dst, bool Cond)
{
    WriteGPR((uint32_t)Cond, Dst);
}

static inline void BRANCHCond(uint16_t Imm, bool Cond)
{
    IsBranching = Cond;
    if (Cond) currTarget = ((uint32_t)Regs.PC.Value + 4) + (int)(((short)Imm) << 2);
}

static inline void BRANCHCondLikely(uint16_t Imm, bool Cond)
{
    BRANCHCond(Imm, Cond);
    if (!Cond) AdvancePC();
}

static inline void JUMPReg(uint8_t Reg)
{
    IsBranching = true;
    currTarget  = ReadGPR(Reg);
}

static inline void JUMPImm(uint32_t Target)
{
    IsBranching = true;
    currTarget  = (Target << 2) | (((uint32_t)Regs.PC.Value + 4) & 0xF0000000);
}

static inline void Link(void)
{
    Regs.GPR[31].Value = (uint32_t)Regs.PC.Value + 8; // Hyah!
}

void Step(void)
{
    bool ShouldBranch = IsBranching;

    uint32_t inst = ReadUInt32((uint32_t)Regs.PC.Value);
    opcode_t op   = OpcodeTable[INST_OP(inst)];
    
    if (op.Interpret == NULL && inst != 0) 
    {
        printf("0x%x\n", INST_OP(inst));
        UndefinedInstError(inst);
        return;
    }

    Regs.GPR[0].Value = 0;

    if (inst != 0) op.Interpret(inst);
    else AdvancePC();

    if (!IsRunning) return;

    if (ShouldBranch) // If we should branch (aka the last instruction was a branch instruction)
    {
        Regs.PC.Value = currTarget; // Then set to the PC the target.
        IsBranching = false; // And reset the variables.
        currTarget = 0;
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

static inline void DSLL(uint32_t Value)
{
    DSLLImm(INST_RT(Value), INST_RD(Value), INST_SA(Value));
    AdvancePC();
}

static inline void DSLLV(uint32_t Value)
{
    DSLLReg(INST_RT(Value), INST_RS(Value), INST_RD(Value));
    AdvancePC();
}

static inline void DSLL32(uint32_t Value)
{
    DSLLImm(INST_RT(Value), INST_RD(Value), INST_SA(Value) + 32);
    AdvancePC();
}

static inline void DSRA(uint32_t Value)
{
    DSRAImm(INST_RT(Value), INST_RD(Value), INST_SA(Value));
    AdvancePC();
}

static inline void DSRAV(uint32_t Value)
{
    DSRAReg(INST_RT(Value), INST_RS(Value), INST_RD(Value));
    AdvancePC();
}

static inline void DSRA32(uint32_t Value)
{
    DSRAImm(INST_RT(Value), INST_RD(Value), INST_SA(Value) + 32);
    AdvancePC();
}

static inline void DSRL(uint32_t Value)
{
    DSRLImm(INST_RT(Value), INST_RD(Value), INST_SA(Value));
    AdvancePC();
}

static inline void DSRLV(uint32_t Value)
{
    DSRLReg(INST_RT(Value), INST_RS(Value), INST_RD(Value));
    AdvancePC();
}

static inline void DSRL32(uint32_t Value)
{
    DSRLImm(INST_RT(Value), INST_RD(Value), INST_SA(Value) + 32);
    AdvancePC();
}

static inline void DSUB(uint32_t Value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    DSUBReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void DSUBU(uint32_t Value)
{
    DSUBReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void JALR(uint32_t Value)
{
    Link();
    JUMPReg(INST_RS(Value));
    AdvancePC();
}

static inline void JR(uint32_t Value)
{
    JUMPReg(INST_RS(Value));
    AdvancePC();
}

static inline void MFHI(uint32_t Value)
{
    WriteGPR(ReadHI(), INST_RD(Value));
    AdvancePC();
}

static inline void MFLO(uint32_t Value)
{
    WriteGPR(ReadLO(), INST_RD(Value));
    AdvancePC();
}

static inline void MTHI(uint32_t Value)
{
    WriteHI(ReadGPR(INST_RS(Value)));
    AdvancePC();
}

static inline void MTLO(uint32_t Value)
{
    WriteLO(ReadGPR(INST_RS(Value)));
    AdvancePC();
}

static inline void MULT(uint32_t Value)
{
    MULTReg(INST_RS(Value), INST_RT(Value));
    AdvancePC();
}

static inline void MULTU(uint32_t Value)
{
    MULTUReg(INST_RS(Value), INST_RT(Value));
    AdvancePC();
}

static inline void NOR(uint32_t Value)
{
    NORReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void OR(uint32_t Value)
{
    ORReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void SLL(uint32_t Value)
{
    SLLImm(INST_RT(Value), INST_RD(Value), INST_SA(Value));
    AdvancePC();
}

static inline void SLLV(uint32_t Value)
{
    SLLReg(INST_RT(Value), INST_RS(Value), INST_RD(Value));
    AdvancePC();
}

static inline void SLT(uint32_t Value)
{
    SETCond(INST_RD(Value), (long)ReadGPR(INST_RS(Value)) < (long)ReadGPR(INST_RT(Value)));
    AdvancePC();
}

static inline void SLTU(uint32_t Value)
{
    SETCond(INST_RD(Value), ReadGPR(INST_RS(Value)) < ReadGPR(INST_RT(Value)));
    AdvancePC();
}

static inline void SRA(uint32_t Value)
{
    SRAImm(INST_RT(Value), INST_RD(Value), INST_SA(Value));
    AdvancePC();
}

static inline void SRAV(uint32_t Value)
{
    SRAReg(INST_RT(Value), INST_RS(Value), INST_RD(Value));
    AdvancePC();
}

static inline void SRL(uint32_t Value)
{
    SRLImm(INST_RT(Value), INST_RD(Value), INST_SA(Value));
    AdvancePC();
}

static inline void SRLV(uint32_t Value)
{
    SRLReg(INST_RT(Value), INST_RS(Value), INST_RD(Value));
    AdvancePC();
}

static inline void SUB(uint32_t Value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    SUBReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void SUBU(uint32_t Value)
{
    SUBReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

static inline void XOR(uint32_t Value)
{
    XORReg(INST_RS(Value), INST_RT(Value), INST_RD(Value));
    AdvancePC();
}

void SPECIAL(uint32_t Value)
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
        case 0b111000: // DSLL
            DSLL(Value);
            return;
        case 0b010100: // DSLLV
            DSLLV(Value);
            return;
        case 0b111100: // DSLL32
            DSLL32(Value);
            return;
        case 0b111011: // DSRA
            DSRA(Value);
            return;
        case 0b010111: // DSRAV
            DSRAV(Value);
            return;
        case 0b111111: // DSRA32
            DSRA32(Value);
            return;
        case 0b111010: // DSRL
            DSRL(Value);
            return;
        case 0b010110: // DSRLV
            DSRLV(Value);
            return;
        case 0b111110: // DSRL32
            DSRL32(Value);
            return;
        case 0b101110: // DSUB
            DSUB(Value);
            return;
        case 0b101111: // DSUBU
            DSUBU(Value);
            return;
        case 0b001001: // JALR
            JALR(Value);
            return;
        case 0b001000: // JR
            JR(Value);
            return;
        case 0b010000: // MFHI
            MFHI(Value);
            return;
        case 0b010010: // MFLO
            MFLO(Value);
            return;
        case 0b010001: // MTHI
            MTHI(Value);
            return;
        case 0b010011: // MTLO
            MTLO(Value);
            return;
        case 0b011000: // MULT
            MULT(Value);
            return;
        case 0b011001: // MULTU
            MULTU(Value);
            return;
        case 0b100111: // NOR
            NOR(Value);
            return;
        case 0b100101: // OR
            OR(Value);
            return;
        case 0b000000: // SLL
            SLL(Value);
            return;
        case 0b000100: // SLLV
            SLLV(Value);
            return;
        case 0b101010: // SLT
            SLT(Value);
            return;
        case 0b101011: // SLTU
            SLTU(Value);
            return;
        case 0b000011: // SRA
            SRA(Value);
            return;
        case 0b000111: // SRAV
            SRAV(Value);
            return;
        case 0b000010: // SRL
            SRL(Value);
            return;
        case 0b000110: // SRLV
            SRLV(Value);
            return;
        case 0b100010: // SUB
            SUB(Value);
            return;
        case 0b100011: // SUBU
            SUBU(Value);
            return;
        case 0b001111: // SYNC
            return; // Executes as a NOP on the VR4300.
        case 0b100110: // XOR
            XOR(Value);
            return;
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

static inline void DMFC0(uint32_t Value)
{
    WriteGPR(ReadCOP0(INST_RD(Value)), INST_RT(Value));
    AdvancePC();
}

static inline void DMTC0(uint32_t Value)
{
    WriteCOP0(ReadGPR(INST_RT(Value)), INST_RD(Value));
    AdvancePC();
}

static inline void ERET(uint32_t Value)
{
    uint32_t NewPC = (Regs.COP0[COP0_Status].Value & 0b100) > 0 
                    ? (uint32_t)Regs.COP0[COP0_ErrorEPC].Value 
                    : (uint32_t)Regs.COP0[COP0_EPC].Value;
    Regs.PC.Value = NewPC;
    Regs.COP0[COP0_Status].Value &= ~0b100;
    Regs.LLbit = false;
}

static inline void MFC0(uint32_t Value)
{
    WriteGPR((uint32_t)ReadCOP0(INST_RD(Value)), INST_RT(Value));
    AdvancePC();
}

static inline void MTC0(uint32_t Value)
{
    WriteCOP0((uint32_t)ReadGPR(INST_RT(Value)), INST_RD(Value));
    AdvancePC();
}

void COPz(uint32_t Value)
{
    if (INST_COP(Value) == 0) // COP0
    {
        switch (INST_RS(Value))
        {
            case 0b00001: // DMFC0
                DMFC0(Value);
                return;
            case 0b00101: // DMTC0
                DMTC0(Value);
                return;
            case 0b10000: // CO
                switch (INST_FUNCT(Value))
                {
                    case 0b011000:
                        ERET(Value);
                        return;
                }
                break;
            case 0b00000: // MFC0
                MFC0(Value);
                return;
            case 0b00100: // MTC0
                MTC0(Value);
                return;
        }
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
    ORImm(INST_RS(Value), INST_IMM(Value), INST_RT(Value));
    AdvancePC();
}

void XORI(uint32_t Value)
{
    XORImm(INST_RS(Value), INST_IMM(Value), INST_RT(Value));
    AdvancePC();
}

void LB(uint32_t Value)
{
    WriteGPR((long)ReadUInt8((uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value)), INST_RT(Value));
    AdvancePC();
}

void LBU(uint32_t Value)
{
    WriteGPR((uint8_t)ReadUInt8((uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value)), INST_RT(Value));
    AdvancePC();
}

void LD(uint32_t Value)
{
    WriteGPR(ReadUInt64((uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value)), INST_RT(Value));
    AdvancePC();
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
    WriteGPR((long)ReadUInt16((uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value)), INST_RT(Value));
    AdvancePC();
}

void LHU(uint32_t Value)
{
    WriteGPR((uint16_t)ReadUInt16((uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value)), INST_RT(Value));
    AdvancePC();
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
    WriteGPR((uint32_t)INST_IMM(Value) << 16, INST_RT(Value));
    AdvancePC();
}

void LW(uint32_t Value)
{
    WriteGPR((long)ReadUInt32((uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value)), INST_RT(Value));
    AdvancePC();
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
    WriteGPR((uint32_t)ReadUInt32((uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value)), INST_RT(Value));
    AdvancePC();
}

void SB(uint32_t Value)
{
    WriteUInt8(ReadGPR(INST_RT(Value)), (uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value));
    AdvancePC();
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
    WriteUInt64(ReadGPR(INST_RT(Value)), (uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value));
    AdvancePC();
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
    WriteUInt16(ReadGPR(INST_RT(Value)), (uint32_t)ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value));
    AdvancePC();
}

void SLTI(uint32_t Value)
{
    SETCond(INST_RT(Value), (long)ReadGPR(INST_RS(Value)) < (short)INST_IMM(Value));
    AdvancePC();
}

void SLTIU(uint32_t Value)
{
    SETCond(INST_RT(Value), ReadGPR(INST_RS(Value)) < (uint16_t)INST_IMM(Value));
    AdvancePC();
}

void SW(uint32_t Value)
{
    WriteUInt32(ReadGPR(INST_RT(Value)), ReadGPR(INST_RS(Value)) + (short)INST_IMM(Value));
    AdvancePC();
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
    BRANCHCond(INST_IMM(Value), (uint32_t)ReadGPR(INST_RS(Value)) == (uint32_t)ReadGPR(INST_RT(Value)));
    AdvancePC();
}

void BEQL(uint32_t Value)
{
    BRANCHCondLikely(INST_IMM(Value), (uint32_t)ReadGPR(INST_RS(Value)) == (uint32_t)ReadGPR(INST_RT(Value)));
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
    BRANCHCond(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) != (int)ReadGPR(INST_RT(Value)));
    AdvancePC();
}

void BNEL(uint32_t Value)
{
    BRANCHCondLikely(INST_IMM(Value), (int)ReadGPR(INST_RS(Value)) != (int)ReadGPR(INST_RT(Value)));
    AdvancePC();
}

void J(uint32_t Value)
{
    JUMPImm(INST_TARGET(Value));
    AdvancePC();
}

void JAL(uint32_t Value)
{
    Link();
    JUMPImm(INST_TARGET(Value));
    AdvancePC();
}

void CACHE(uint32_t Value)
{
    AdvancePC(); // Does nothing (we don't need to emulate this)
}