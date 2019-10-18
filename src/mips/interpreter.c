#include "mips/interpreter.h"

#include "common.h"

__attribute__((__always_inline__)) static inline void ADD_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR((long)((uint32_t)read_GPR(reg1, cpu) + (uint32_t)read_GPR(reg2, cpu)), dst, cpu);
}

__attribute__((__always_inline__)) static inline void ADD_imm(uint8_t reg, uint16_t imm, uint8_t dst, cpu_t* cpu)
{
    write_GPR((long)((uint32_t)read_GPR(reg, cpu) + (short)imm), dst, cpu);
}

__attribute__((__always_inline__)) static inline void SUB_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR((long)((uint32_t)read_GPR(reg1, cpu) - (uint32_t)read_GPR(reg2, cpu)), dst, cpu);
}

__attribute__((__always_inline__)) static inline void DIV_reg(uint8_t reg1, uint8_t reg2, cpu_t* cpu)
{
    if ((int)read_GPR(reg2, cpu))
    {
        write_LO((int)read_GPR(reg1, cpu) / (long)((int)read_GPR(reg2, cpu)), cpu);
        write_HI((int)read_GPR(reg1, cpu) % (long)((int)read_GPR(reg2, cpu)), cpu);
    }
    else
    {
        if ((int)read_GPR(reg1, cpu) > 0)
            write_LO(0xFFFFFFFF, cpu);
        else if ((int)read_GPR(reg1, cpu) < 0)
            write_LO(0x00000001, cpu);

        write_HI((int)read_GPR(reg1, cpu), cpu);
    }
}

__attribute__((__always_inline__)) static inline void DIVU_reg(uint8_t reg1, uint8_t reg2, cpu_t* cpu)
{
    if ((uint32_t)read_GPR(reg2, cpu))
    {
        write_LO((uint32_t)read_GPR(reg1, cpu) / (uint64_t)((uint32_t)read_GPR(reg2, cpu)), cpu);
        write_HI((uint32_t)read_GPR(reg1, cpu) % (uint64_t)((uint32_t)read_GPR(reg2, cpu)), cpu);
    }
    else
    {
        write_LO(0xFFFFFFFF, cpu);
        write_HI((uint32_t)read_GPR(reg1, cpu), cpu);
    }
}

__attribute__((__always_inline__)) static inline void MULT_reg(uint8_t reg1, uint8_t reg2, cpu_t* cpu)
{
    long res = (int)read_GPR(reg1, cpu) * (long)((int)read_GPR(reg2, cpu));
    write_LO(res & 0xFFFFFFFF, cpu);
    write_HI(res >> 32, cpu);
}

__attribute__((__always_inline__)) static inline void MULTU_reg(uint8_t reg1, uint8_t reg2, cpu_t* cpu)
{
    uint64_t res = (uint32_t)read_GPR(reg1, cpu) * (uint64_t)((uint32_t)read_GPR(reg2, cpu));
    write_LO(res & 0xFFFFFFFF, cpu);
    write_HI((long)res >> 32, cpu);
}

__attribute__((__always_inline__)) static inline void SLL_imm(uint8_t reg, uint8_t dst, uint8_t sa, cpu_t* cpu)
{
    write_GPR(read_GPR(reg, cpu) << sa, dst, cpu);
}

__attribute__((__always_inline__)) static inline void SLL_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR((uint32_t)read_GPR(reg1, cpu) << (read_GPR(reg2, cpu) & 0x1F), dst, cpu);
}

__attribute__((__always_inline__)) static inline void SRA_imm(uint8_t reg, uint8_t dst, uint8_t sa, cpu_t* cpu)
{
    write_GPR((long)((int)read_GPR(reg, cpu) >> sa), dst, cpu);
}

__attribute__((__always_inline__)) static inline void SRA_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR((long)((int)read_GPR(reg1, cpu) >> (read_GPR(reg2, cpu) & 0x1F)), dst, cpu);
}

__attribute__((__always_inline__)) static inline void SRL_imm(uint8_t reg, uint8_t dst, uint8_t sa, cpu_t* cpu)
{
    write_GPR((uint32_t)read_GPR(reg, cpu) >> sa, dst, cpu);
}

__attribute__((__always_inline__)) static inline void SRL_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg1, cpu) >> (read_GPR(reg2, cpu) & 0x1F), dst, cpu);
}

__attribute__((__always_inline__)) static inline void DADD_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg1, cpu) + read_GPR(reg2, cpu), dst, cpu);
}

__attribute__((__always_inline__)) static inline void DADD_imm(uint8_t reg1, uint16_t imm, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg1, cpu) + (short)imm, dst, cpu);
}

__attribute__((__always_inline__)) static inline void DSUB_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg1, cpu) - read_GPR(reg2, cpu), dst, cpu);
}

__attribute__((__always_inline__)) static inline void DDIV_reg(uint8_t reg1, uint8_t reg2, cpu_t* cpu)
{
    if ((long)read_GPR(reg2, cpu))
    {
        write_LO((long)read_GPR(reg1, cpu) / (__int128_t)((long)read_GPR(reg2, cpu)), cpu);
        write_HI((long)read_GPR(reg1, cpu) % (__int128_t)((long)read_GPR(reg2, cpu)), cpu);
    }
    else
    {
        if ((long)read_GPR(reg1, cpu) > 0)
            write_LO(0xFFFFFFFFFFFFFFFF, cpu);
        else if ((long)read_GPR(reg1, cpu) < 0)
            write_LO(0x0000000000000001, cpu);

        write_HI((long)read_GPR(reg1, cpu), cpu);
    }
}

__attribute__((__always_inline__)) static inline void DDIVU_reg(uint8_t reg1, uint8_t reg2, cpu_t* cpu)
{
    if ((uint64_t)read_GPR(reg2, cpu))
    {
        write_LO((uint64_t)read_GPR(reg1, cpu) / (__uint128_t)((uint64_t)read_GPR(reg2, cpu)), cpu);
        write_HI((uint64_t)read_GPR(reg1, cpu) % (__uint128_t)((uint64_t)read_GPR(reg2, cpu)), cpu);
    }
    else
    {
        write_LO(0xFFFFFFFFFFFFFFFF, cpu);
        write_HI((uint64_t)read_GPR(reg1, cpu), cpu);
    }
}

__attribute__((__always_inline__)) static inline void DMULT_reg(uint8_t reg1, uint8_t reg2, cpu_t* cpu)
{
    __int128_t res = (long)read_GPR(reg1, cpu) * (__int128_t)((long)read_GPR(reg2, cpu));
    write_LO(res, cpu);
    write_HI(res >> 64, cpu);
}

__attribute__((__always_inline__)) static inline void DMULTU_reg(uint8_t reg1, uint8_t reg2, cpu_t* cpu)
{
    if ((uint64_t)read_GPR(reg2, cpu) == 0xFFFFFFFF)
    {
        write_LO(0x0000000000000001, cpu);
        write_HI(0xFFFFFFFFFFFFFFFE, cpu);
    }
    else if ((uint64_t)read_GPR(reg1, cpu) == 0xFFFFFFFF)
    {
        write_LO(0xFFFFFFFFFFFFFFFE, cpu);
        write_HI(0x0000000000000001, cpu);
    }
    else
    {
        __uint128_t res = (uint64_t)read_GPR(reg1, cpu) * (__uint128_t)((uint64_t)read_GPR(reg2, cpu));
        write_LO(res, cpu);
        write_HI((__int128_t)res >> 64, cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSLL_imm(uint8_t reg, uint8_t dst, uint8_t sa, cpu_t* cpu)
{
    write_GPR(read_GPR(reg, cpu) << sa, dst, cpu);
}

__attribute__((__always_inline__)) static inline void DSLL_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg1, cpu) << (read_GPR(reg2, cpu) & 0x3F), dst, cpu);
}

__attribute__((__always_inline__)) static inline void DSRA_imm(uint8_t reg, uint8_t dst, uint8_t sa, cpu_t* cpu)
{
    write_GPR((long)read_GPR(reg, cpu) >> sa, dst, cpu);
}

__attribute__((__always_inline__)) static inline void DSRA_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR((long)read_GPR(reg1, cpu) >> (read_GPR(reg2, cpu) & 0x3F), dst, cpu);
}

__attribute__((__always_inline__)) static inline void DSRL_imm(uint8_t reg, uint8_t dst, uint8_t sa, cpu_t* cpu)
{
    write_GPR(read_GPR(reg, cpu) >> sa, dst, cpu);
}

__attribute__((__always_inline__)) static inline void DSRL_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg1, cpu) >> (read_GPR(reg2, cpu) & 0x3F), dst, cpu);
}

__attribute__((__always_inline__)) static inline void AND_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg1, cpu) & read_GPR(reg2, cpu), dst, cpu);
}

__attribute__((__always_inline__)) static inline void AND_imm(uint8_t reg, uint16_t imm, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg, cpu) & imm, dst, cpu);
}

__attribute__((__always_inline__)) static inline void OR_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg1, cpu) | read_GPR(reg2, cpu), dst, cpu);
}

__attribute__((__always_inline__)) static inline void OR_imm(uint8_t reg, uint16_t imm, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg, cpu) | imm, dst, cpu);
}

__attribute__((__always_inline__)) static inline void XOR_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg1, cpu) ^ read_GPR(reg2, cpu), dst, cpu);
}

__attribute__((__always_inline__)) static inline void XOR_imm(uint8_t reg, uint16_t imm, uint8_t dst, cpu_t* cpu)
{
    write_GPR(read_GPR(reg, cpu) ^ imm, dst, cpu);
}

__attribute__((__always_inline__)) static inline void NOR_reg(uint8_t reg1, uint8_t reg2, uint8_t dst, cpu_t* cpu)
{
    write_GPR(~(read_GPR(reg1, cpu) | read_GPR(reg2, cpu)), dst, cpu);
}

__attribute__((__always_inline__)) static inline void SET_cond(uint8_t dst, bool Cond, cpu_t* cpu)
{
    write_GPR((uint32_t)Cond, dst, cpu);
}

__attribute__((__always_inline__)) static inline void BRANCH_cond(uint16_t imm, bool Cond, cpu_t* cpu)
{
    cpu->is_branching = Cond;
    if (Cond) cpu->curr_target = ((uint32_t)cpu->regs.PC.value + 4) + (int)(((short)imm) << 2);
}

__attribute__((__always_inline__)) static inline void BRANCH_cond_likely(uint16_t imm, bool Cond, cpu_t* cpu)
{
    BRANCH_cond(imm, Cond, cpu);
    if (!Cond) advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TRAP_cond(bool Cond, cpu_t* cpu)
{
    if (Cond) invoke_trap();
}

__attribute__((__always_inline__)) static inline void JUMP_reg(uint8_t reg, cpu_t* cpu)
{
    cpu->is_branching = true;
    cpu->curr_target  = read_GPR(reg, cpu);
}

__attribute__((__always_inline__)) static inline void JUMP_imm(uint32_t target, cpu_t* cpu)
{
    cpu->is_branching = true;
    cpu->curr_target  = (target << 2) | (((uint32_t)cpu->regs.PC.value + 4) & 0xF0000000);
}

__attribute__((__always_inline__)) static inline void link_PC(cpu_t* cpu)
{
    cpu->regs.GPR[31].value = (uint32_t)cpu->regs.PC.value + 8; // Hyah!
}

__attribute__((__always_inline__)) static inline void ADD(uint32_t value, cpu_t* cpu)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    ADD_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void ADDU(uint32_t value, cpu_t* cpu)
{
    ADD_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void AND(uint32_t value, cpu_t* cpu)
{
    AND_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BREAK(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        SP_STATUS_REG_R |= bswap_32(0x00000002 | 0x00000001); // Set the broke and halt flags.
        if (bswap_32(SP_STATUS_REG_R) & 0x00000040)
            invoke_mi_interrupt(MI_INTR_SP);
    }
    else
    {
        invoke_break();
    }
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void DADD(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
        DADD_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DADDU(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DADD_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DDIV(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DDIV_reg(INST_RS(value), INST_RT(value), cpu);
        cpu->curr_inst_cycles = 69;
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DDIVU(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DDIVU_reg(INST_RS(value), INST_RT(value), cpu);
        cpu->curr_inst_cycles = 69;
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DIV(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DIV_reg(INST_RS(value), INST_RT(value), cpu);
        cpu->curr_inst_cycles = 37;
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DIVU(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DIVU_reg(INST_RS(value), INST_RT(value), cpu);
        cpu->curr_inst_cycles = 37;
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DMULT(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DMULT_reg(INST_RS(value), INST_RT(value), cpu);
        cpu->curr_inst_cycles = 8;
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DMULTU(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DMULTU_reg(INST_RS(value), INST_RT(value), cpu);
        cpu->curr_inst_cycles = 8;
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSLL(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSLL_imm(INST_RT(value), INST_RD(value), INST_SA(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSLLV(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSLL_reg(INST_RT(value), INST_RS(value), INST_RD(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSLL32(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSLL_imm(INST_RT(value), INST_RD(value), INST_SA(value) + 32, cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSRA(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSRA_imm(INST_RT(value), INST_RD(value), INST_SA(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSRAV(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSRA_reg(INST_RT(value), INST_RS(value), INST_RD(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSRA32(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSRA_imm(INST_RT(value), INST_RD(value), INST_SA(value) + 32, cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSRL(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSRL_imm(INST_RT(value), INST_RD(value), INST_SA(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSRLV(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSRL_reg(INST_RT(value), INST_RS(value), INST_RD(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSRL32(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSRL_imm(INST_RT(value), INST_RD(value), INST_SA(value) + 32, cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSUB(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
        DSUB_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void DSUBU(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        DSUB_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void JALR(uint32_t value, cpu_t* cpu)
{
    link_PC(cpu);
    JUMP_reg(INST_RS(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void JR(uint32_t value, cpu_t* cpu)
{
    JUMP_reg(INST_RS(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void MFHI(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        write_GPR(read_HI(cpu), INST_RD(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void MFLO(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        write_GPR(read_LO(cpu), INST_RD(value), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void MTHI(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        write_HI(read_GPR(INST_RS(value), cpu), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void MTLO(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        write_LO(read_GPR(INST_RS(value), cpu), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void MULT(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        MULT_reg(INST_RS(value), INST_RT(value), cpu);
        cpu->curr_inst_cycles = 5;
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void MULTU(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        MULTU_reg(INST_RS(value), INST_RT(value), cpu);
        cpu->curr_inst_cycles = 5;
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void NOR(uint32_t value, cpu_t* cpu)
{
    NOR_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void OR(uint32_t value, cpu_t* cpu)
{
    OR_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SLL(uint32_t value, cpu_t* cpu)
{
    SLL_imm(INST_RT(value), INST_RD(value), INST_SA(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SLLV(uint32_t value, cpu_t* cpu)
{
    SLL_reg(INST_RT(value), INST_RS(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SLT(uint32_t value, cpu_t* cpu)
{
    SET_cond(INST_RD(value), (int)read_GPR(INST_RS(value), cpu) < (int)read_GPR(INST_RT(value), cpu), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SLTU(uint32_t value, cpu_t* cpu)
{
    SET_cond(INST_RD(value), (uint32_t)read_GPR(INST_RS(value), cpu) < (uint32_t)read_GPR(INST_RT(value), cpu), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SRA(uint32_t value, cpu_t* cpu)
{
    SRA_imm(INST_RT(value), INST_RD(value), INST_SA(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SRAV(uint32_t value, cpu_t* cpu)
{
    SRA_reg(INST_RT(value), INST_RS(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SRL(uint32_t value, cpu_t* cpu)
{
    SRL_imm(INST_RT(value), INST_RD(value), INST_SA(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SRLV(uint32_t value, cpu_t* cpu)
{
    SRL_reg(INST_RT(value), INST_RS(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SUB(uint32_t value, cpu_t* cpu)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    SUB_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SUBU(uint32_t value, cpu_t* cpu)
{
    SUB_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TEQ(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        TRAP_cond((uint32_t)read_GPR(INST_RS(value), cpu) == (uint32_t)read_GPR(INST_RT(value), cpu), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void TGE(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        TRAP_cond((int)read_GPR(INST_RS(value), cpu) >= (int)read_GPR(INST_RT(value), cpu), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void TGEU(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        TRAP_cond((uint32_t)read_GPR(INST_RS(value), cpu) >= (uint32_t)read_GPR(INST_RT(value), cpu), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void TLT(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        TRAP_cond((int)read_GPR(INST_RS(value), cpu) < (int)read_GPR(INST_RT(value), cpu), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void TLTU(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        TRAP_cond((uint32_t)read_GPR(INST_RS(value), cpu) < (uint32_t)read_GPR(INST_RT(value), cpu), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void TNE(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        TRAP_cond((uint32_t)read_GPR(INST_RS(value), cpu) != (uint32_t)read_GPR(INST_RT(value), cpu), cpu);
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void XOR(uint32_t value, cpu_t* cpu)
{
    XOR_reg(INST_RS(value), INST_RT(value), INST_RD(value), cpu);
    advance_PC(cpu);
}

void SPECIAL(uint32_t value, cpu_t* cpu)
{
    switch (INST_FUNCT(value))
    {
        case 0b100000: // ADD
            ADD(value, cpu);
            return;
        case 0b100001: // ADDU
            ADDU(value, cpu);
            return;
        case 0b100100: // AND
            AND(value, cpu);
            return;
        case 0b001101: // BREAK
            BREAK(value, cpu);
            return;
        case 0b101100: // DADD
            DADD(value, cpu);
            return;
        case 0b101101: // DADDU
            DADDU(value, cpu);
            return;
        case 0b011110: // DDIV
            DDIV(value, cpu);
            return;
        case 0b011111: // DDIVU
            DDIVU(value, cpu);
            return;
        case 0b011010: // DIV
            DIV(value, cpu);
            return;
        case 0b011011: // DIVU
            DIVU(value, cpu);
            return;
        case 0b011100: // DMULT
            DMULT(value, cpu);
            return;
        case 0b011101: // DMULT
            DMULTU(value, cpu);
            return;
        case 0b111000: // DSLL
            DSLL(value, cpu);
            return;
        case 0b010100: // DSLLV
            DSLLV(value, cpu);
            return;
        case 0b111100: // DSLL32
            DSLL32(value, cpu);
            return;
        case 0b111011: // DSRA
            DSRA(value, cpu);
            return;
        case 0b010111: // DSRAV
            DSRAV(value, cpu);
            return;
        case 0b111111: // DSRA32
            DSRA32(value, cpu);
            return;
        case 0b111010: // DSRL
            DSRL(value, cpu);
            return;
        case 0b010110: // DSRLV
            DSRLV(value, cpu);
            return;
        case 0b111110: // DSRL32
            DSRL32(value, cpu);
            return;
        case 0b101110: // DSUB
            DSUB(value, cpu);
            return;
        case 0b101111: // DSUBU
            DSUBU(value, cpu);
            return;
        case 0b001001: // JALR
            JALR(value, cpu);
            return;
        case 0b001000: // JR
            JR(value, cpu);
            return;
        case 0b010000: // MFHI
            MFHI(value, cpu);
            return;
        case 0b010010: // MFLO
            MFLO(value, cpu);
            return;
        case 0b010001: // MTHI
            MTHI(value, cpu);
            return;
        case 0b010011: // MTLO
            MTLO(value, cpu);
            return;
        case 0b011000: // MULT
            MULT(value, cpu);
            return;
        case 0b011001: // MULTU
            MULTU(value, cpu);
            return;
        case 0b100111: // NOR
            NOR(value, cpu);
            return;
        case 0b100101: // OR
            OR(value, cpu);
            return;
        case 0b000000: // SLL
            SLL(value, cpu);
            return;
        case 0b000100: // SLLV
            SLLV(value, cpu);
            return;
        case 0b101010: // SLT
            SLT(value, cpu);
            return;
        case 0b101011: // SLTU
            SLTU(value, cpu);
            return;
        case 0b000011: // SRA
            SRA(value, cpu);
            return;
        case 0b000111: // SRAV
            SRAV(value, cpu);
            return;
        case 0b000010: // SRL
            SRL(value, cpu);
            return;
        case 0b000110: // SRLV
            SRLV(value, cpu);
            return;
        case 0b100010: // SUB
            SUB(value, cpu);
            return;
        case 0b100011: // SUBU
            SUBU(value, cpu);
            return;
        case 0b001111: // SYNC
            if (cpu->rsp)
            {
                MIPS_undefined_inst_error(value, cpu);
            }
            else
            {
                advance_PC(cpu); // Executes as a NOP on the VR4300.
            }
            return;
        case 0b110100: // TEQ
            TEQ(value, cpu);
            return;
        case 0b110000: // TGE
            TGE(value, cpu);
            return;
        case 0b110001: // TGEU
            TGEU(value, cpu);
            return;
        case 0b110010: // TLT
            TLT(value, cpu);
            return;
        case 0b110011: // TLTU
            TLTU(value, cpu);
            return;
        case 0b110110: // TNE
            TNE(value, cpu);
            return;
        case 0b100110: // XOR
            XOR(value, cpu);
            return;
    }
    MIPS_undefined_inst_error(value, cpu);
}

__attribute__((__always_inline__)) static inline void BGEZ(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) >= 0, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BGEZAL(uint32_t value, cpu_t* cpu)
{
    link_PC(cpu);
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) >= 0, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BGEZALL(uint32_t value, cpu_t* cpu)
{
    link_PC(cpu);
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) >= 0, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BGEZL(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) >= 0, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BLTZ(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) < 0, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BLTZAL(uint32_t value, cpu_t* cpu)
{
    link_PC(cpu);
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) < 0, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BLTZALL(uint32_t value, cpu_t* cpu)
{
    link_PC(cpu);
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) < 0, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BLTZL(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) < 0, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TEQI(uint32_t value, cpu_t* cpu)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value), cpu) == (int)INST_IMM(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TGEI(uint32_t value, cpu_t* cpu)
{
    TRAP_cond((int)read_GPR(INST_RS(value), cpu) >= (int)INST_IMM(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TGEIU(uint32_t value, cpu_t* cpu)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value), cpu) >= (uint32_t)INST_IMM(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TLTI(uint32_t value, cpu_t* cpu)
{
    TRAP_cond((int)read_GPR(INST_RS(value), cpu) < (int)INST_IMM(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TLTIU(uint32_t value, cpu_t* cpu)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value), cpu) < (uint32_t)INST_IMM(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TNEI(uint32_t value, cpu_t* cpu)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value), cpu) != (int)INST_IMM(value), cpu);
    advance_PC(cpu);
}

void REGIMM(uint32_t value, cpu_t* cpu)
{
    switch (INST_RT(value))
    {
        case 0b00001: // BGEZ
            BGEZ(value, cpu);
            return;
        case 0b10001: // BGEZAL
            BGEZAL(value, cpu);
            return;
        case 0b10011: // BGEZALL
            BGEZALL(value, cpu);
            return;
        case 0b00011: // BGEZL
            BGEZL(value, cpu);
            return;
        case 0b00000: // BLTZ
            BLTZ(value, cpu);
            return;
        case 0b10000: // BLTZAL
            BLTZAL(value, cpu);
            return;
        case 0b10010: // BLTZALL
            BLTZALL(value, cpu);
            return;
        case 0b00010: // BLTZL
            BLTZL(value, cpu);
            return;
        case 0b01100: // TEQI
            TEQI(value, cpu);
            return;
        case 0b01000: // TGEI
            TGEI(value, cpu);
            return;
        case 0b01001: // TGEIU
            TGEIU(value, cpu);
            return;
        case 0b01010: // TLTI
            TLTI(value, cpu);
            return;
        case 0b01011: // TLTIU
            TLTIU(value, cpu);
            return;
        case 0b01110: // TNEI
            TNEI(value, cpu);
            return;
    }
    MIPS_undefined_inst_error(value, cpu);
}

__attribute__((__always_inline__)) static inline void DMFC0(uint32_t value, cpu_t* cpu)
{
    write_GPR(read_COP0(INST_RD(value), cpu), INST_RT(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void DMTC0(uint32_t value, cpu_t* cpu)
{
    write_COP0(read_GPR(INST_RT(value), cpu), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void ERET(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    else
    {
        uint32_t NewPC = (cpu->regs.COP0[COP0_STATUS].value & 0b100) > 0 
                        ? (uint32_t)cpu->regs.COP0[COP0_ERROREPC].value 
                        : (uint32_t)cpu->regs.COP0[COP0_EPC].value;
        cpu->regs.PC.value = NewPC;
        cpu->regs.COP0[COP0_STATUS].value &= (cpu->regs.COP0[COP0_STATUS].value & 0b100) > 0 ? ~0b100 : ~0b010;
        cpu->regs.LLbit = false;
    }
}

__attribute__((__always_inline__)) static inline void MFC0(uint32_t value, cpu_t* cpu)
{
    write_GPR((uint32_t)read_COP0(INST_RD(value), cpu), INST_RT(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void MTC0(uint32_t value, cpu_t* cpu)
{
    write_COP0((uint32_t)read_GPR(INST_RT(value), cpu), INST_RD(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TLBP(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
        return;
    }
    else
    {
        probe_TLB();
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void TLBR(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
        return;
    }
    else
    {
        read_TLB_entry();
    }
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TLBWI(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
        return;
    }
    else
    {
        write_TLB_entry_indexed();
        advance_PC(cpu);
    }
}

__attribute__((__always_inline__)) static inline void TLBWR(uint32_t value, cpu_t* cpu)
{
    if (cpu->rsp)
    {
        MIPS_undefined_inst_error(value, cpu);
        return;
    }
    else
    {
        write_TLB_entry_random();
        advance_PC(cpu);
    }
}

void COP0(uint32_t value, cpu_t* cpu)
{
    switch (INST_RS(value))
    {
        case 0b00001: // DMFC0
            DMFC0(value, cpu);
            return;
        case 0b00101: // DMTC0
            DMTC0(value, cpu);
            return;
        case 0b10000: // CO
            switch (INST_FUNCT(value))
            {
                case 0b011000: // ERET
                    ERET(value, cpu);
                    return;
                case 0b001000: // TLBP
                    TLBP(value, cpu);
                    return;
                case 0b000001: // TLBR
                    TLBR(value, cpu);
                    return;
                case 0b000010: // TLBWI
                    TLBWI(value, cpu);
                    return;
                case 0b000110: // TLBWR
                    TLBWR(value, cpu);
                    return;
            }
            break;
        case 0b00000: // MFC0
            MFC0(value, cpu);
            return;
        case 0b00100: // MTC0
            MTC0(value, cpu);
            return;
    }
    MIPS_undefined_inst_error(value, cpu);
}

__attribute__((__always_inline__)) static inline void ABS_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        if (isnan(float_fs))
        {
            // TODO: Invalid Operation Exception
            advance_PC(cpu);
            return;
        }

        float_fs = fabsf(float_fs);
        fd = *(uint64_t*)&float_fs;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        if (isnan(double_fs))
        {
            // TODO: Invalid Operation Exception
            advance_PC(cpu);
            return;
        }

        double_fs = fabs(double_fs);
        fd = *(uint64_t*)&double_fs;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void ADD_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t ft = read_FPR(INST_FT(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;
        float float_ft = *(float*)&ft;

        float_fs += float_ft;
        fd = *(uint64_t*)&float_fs;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;
        double double_ft = *(double*)&ft;

        double_fs += double_ft;
        fd = *(uint64_t*)&double_fs;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BC1F(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond(INST_IMM(value), !cpu->regs.COC1, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BC1FL(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond_likely(INST_IMM(value), !cpu->regs.COC1, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BC1T(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond(INST_IMM(value), cpu->regs.COC1, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void BC1TL(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond_likely(INST_IMM(value), cpu->regs.COC1, cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void C_cond_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t ft = read_FPR(INST_FT(value), cpu);
    bool less      = false;
    bool equal     = false;
    bool unordered = false;
    uint8_t cond   = INST_FUNCT(value) & 0b001111;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;
        float float_ft = *(float*)&ft;

        if (isnan(float_fs) || isnan(float_ft))
        {
            less      = false;
            equal     = false;
            unordered = true;
            if (cond & 0b1000)
            {
                // TODO: Invalid Operation Exception
                advance_PC(cpu);
                return;
            }
        }
        else
        {
            less      = float_fs < float_ft;
            equal     = float_fs == float_ft;
            unordered = false;
        }
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;
        double double_ft = *(double*)&ft;

        if (isnan(double_fs) || isnan(double_ft))
        {
            less      = false;
            equal     = false;
            unordered = true;
            if (cond & 0b1000)
            {
                // TODO: Invalid Operation Exception
                advance_PC(cpu);
                return;
            }
        }
        else
        {
            less      = double_fs < double_ft;
            equal     = double_fs == double_ft;
            unordered = false;
        }
    }
    else
        MIPS_undefined_inst_error(value, cpu);

    bool condition = ((cond & 0b0100) && less) || ((cond & 0b0010) && equal) || ((cond & 0b0001) && unordered);
    cpu->regs.COC1 = condition;
    write_FPR(read_FPR(31, cpu) | (condition << 23), 31, cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void CEIL_L_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = ceilf(float_fs);
        long res = (long)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = ceil(double_fs);
        long res = (long)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void CEIL_W_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = ceilf(float_fs);
        int res = (int)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = ceil(double_fs);
        int res = (int)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void CFC1(uint32_t value, cpu_t* cpu)
{
    if (INST_RD(value) == 31 || INST_RD(value) == 0)
    {
        write_GPR((uint32_t)read_FPR(INST_RD(value), cpu), INST_RT(value), cpu);
    }
    else
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void CTC1(uint32_t value, cpu_t* cpu)
{
    if (INST_RD(value) == 31 || INST_RD(value) == 0)
    {
        write_FPR((uint32_t)read_GPR(INST_RT(value), cpu), INST_RD(value), cpu);
    }
    else
    {
        MIPS_undefined_inst_error(value, cpu);
    }
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void CVT_D_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        double res = (double)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 20) // Word
    {
        int word_fs = *(int*)&fs;

        double res = (double)word_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 21) // Long
    {
        long long_fs = *(long*)&fs;

        double res = (double)long_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);

    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void CVT_L_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        long res = (long)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        long res = (long)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);

    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void CVT_S_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        float res = (float)double_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 20) // Word
    {
        int word_fs = *(int*)&fs;

        float res = (float)word_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 21) // Long
    {
        long long_fs = *(long*)&fs;

        float res = (float)long_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);

    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void CVT_W_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        int res = (int)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        int res = (int)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);

    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void DIV_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t ft = read_FPR(INST_FT(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;
        float float_ft = *(float*)&ft;

        if (float_ft == 0)
        {
            // TODO: Division By Zero Exception
            advance_PC(cpu);
            return;
        }

        float_fs /= float_ft;
        fd = *(uint64_t*)&float_fs;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;
        double double_ft = *(double*)&ft;

        if (double_ft == 0)
        {
            // TODO: Division By Zero Exception
            advance_PC(cpu);
            return;
        }

        double_fs /= double_ft;
        fd = *(uint64_t*)&double_fs;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void DMFC1(uint32_t value, cpu_t* cpu)
{
    write_GPR(read_FPR(INST_FS(value), cpu), INST_RT(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void DMTC1(uint32_t value, cpu_t* cpu)
{
    write_FPR(read_GPR(INST_RT(value), cpu), INST_FS(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void FLOOR_L_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = floorf(float_fs);
        long res = (long)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = floor(double_fs);
        long res = (long)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void FLOOR_W_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = floorf(float_fs);
        int res = (int)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = floor(double_fs);
        int res = (int)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void MFC1(uint32_t value, cpu_t* cpu)
{
    write_GPR((uint32_t)read_FPR(INST_FS(value), cpu), INST_RT(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void MOV_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        fd = *(uint64_t*)&float_fs;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        fd = *(uint64_t*)&double_fs;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void MTC1(uint32_t value, cpu_t* cpu)
{
    write_FPR((uint32_t)read_GPR(INST_RT(value), cpu), INST_FS(value), cpu);
    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void MUL_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t ft = read_FPR(INST_FT(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;
        float float_ft = *(float*)&ft;

        float_fs *= float_ft;
        fd = *(uint64_t*)&float_fs;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;
        double double_ft = *(double*)&ft;

        double_fs *= double_ft;
        fd = *(uint64_t*)&double_fs;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void NEG_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = -float_fs;
        fd = *(uint64_t*)&float_fs;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = -double_fs;
        fd = *(uint64_t*)&double_fs;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void ROUND_L_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = roundf(float_fs);
        long res = (long)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = round(double_fs);
        long res = (long)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void ROUND_W_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = roundf(float_fs);
        int res = (int)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = round(double_fs);
        int res = (int)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SQRT_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = sqrtf(float_fs);
        fd = *(uint64_t*)&float_fs;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = sqrt(double_fs);
        fd = *(uint64_t*)&double_fs;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void SUB_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t ft = read_FPR(INST_FT(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;
        float float_ft = *(float*)&ft;

        float_fs -= float_ft;
        fd = *(uint64_t*)&float_fs;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;
        double double_ft = *(double*)&ft;

        double_fs -= double_ft;
        fd = *(uint64_t*)&double_fs;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TRUNC_L_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = truncf(float_fs);
        long res = (long)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = trunc(double_fs);
        long res = (long)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

__attribute__((__always_inline__)) static inline void TRUNC_W_fmt(uint32_t value, cpu_t* cpu)
{
    uint64_t fs = read_FPR(INST_FS(value), cpu);
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        float_fs = truncf(float_fs);
        int res = (int)float_fs;
        fd = *(uint64_t*)&res;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        double_fs = trunc(double_fs);
        int res = (int)double_fs;
        fd = *(uint64_t*)&res;
    }
    else
        MIPS_undefined_inst_error(value, cpu);
    
    write_FPR(fd, INST_FD(value), cpu);

    advance_PC(cpu);
}

void COP1(uint32_t value, cpu_t* cpu)
{
    if (INST_FUNCT(value) == 0 && INST_SA(value) == 0)
    {
        switch (INST_RS(value))
        {
            case 0b00010: // CFC1
                CFC1(value, cpu);
                return;
            case 0b00110: // CTC1
                CTC1(value, cpu);
                return;
            case 0b00001: // DMFC1
                DMFC1(value, cpu);
                return;
            case 0b00101: // DMTC1
                DMTC1(value, cpu);
                return;
            case 0b00000: // MFC1
                MFC1(value, cpu);
                return;
            case 0b00100: // MTC1
                MTC1(value, cpu);
                return;
        }
    }
    else if (INST_RS(value) == 0b01000)
    {
        switch (INST_RT(value))
        {
            case 0b00000: // BC1F
                BC1F(value, cpu);
                return;
            case 0b00010: // BC1FL
                BC1FL(value, cpu);
                return;
            case 0b00001: // BC1T
                BC1T(value, cpu);
                return;
            case 0b00011: // BC1TL
                BC1TL(value, cpu);
                return;
        }
    }

    switch (INST_FUNCT(value))
    {
        case 0b000101: // ABS.fmt
            ABS_fmt(value, cpu);
            return;
        case 0b000000: // ADD.fmt
            ADD_fmt(value, cpu);
            return;
        case 0b001010: // CEIL.L.fmt
            CEIL_L_fmt(value, cpu);
            return;
        case 0b001110: // CEIL.W.fmt
            CEIL_W_fmt(value, cpu);
            return;
        case 0b100001: // CVT.D.fmt
            CVT_D_fmt(value, cpu);
            return;
        case 0b100101: // CVT.L.fmt
            CVT_L_fmt(value, cpu);
            return;
        case 0b100000: // CVT.S.fmt
            CVT_S_fmt(value, cpu);
            return;
        case 0b100100: // CVT.W.fmt
            CVT_W_fmt(value, cpu);
            return;
        case 0b000011: // DIV.fmt
            DIV_fmt(value, cpu);
            return;
        case 0b001011: // FLOOR.L.fmt
            FLOOR_L_fmt(value, cpu);
            return;
        case 0b001111: // FLOOR.W.fmt
            FLOOR_W_fmt(value, cpu);
            return;
        case 0b000110: // MOV.fmt
            MOV_fmt(value, cpu);
            return;
        case 0b000010: // MUL.fmt
            MUL_fmt(value, cpu);
            return;
        case 0b000111: // NEG.fmt
            NEG_fmt(value, cpu);
            return;
        case 0b001000: // ROUND.L.fmt
            ROUND_L_fmt(value, cpu);
            return;
        case 0b001100: // ROUND.W.fmt
            ROUND_W_fmt(value, cpu);
            return;
        case 0b000100: // SQRT.fmt
            SQRT_fmt(value, cpu);
            return;
        case 0b000001: // SUB.fmt
            SUB_fmt(value, cpu);
            return;
        case 0b001001: // TRUNC.L.fmt
            TRUNC_L_fmt(value, cpu);
            return;
        case 0b001101: // TRUNC.W.fmt
            TRUNC_W_fmt(value, cpu);
            return;
    }

    if ((INST_FUNCT(value) & 0b110000) == 0b110000 && INST_SA(value) == 0) // C.cond.fmt
    {
        C_cond_fmt(value, cpu);
        return;
    }
    
    MIPS_undefined_inst_error(value, cpu);
}

void LDC1(uint32_t value, cpu_t* cpu)
{
    cpu->curr_inst_cycles = 5;
    write_FPR(read_uint64((uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value)), INST_FT(value), cpu);
    advance_PC(cpu);
}

void LWC1(uint32_t value, cpu_t* cpu)
{
    cpu->curr_inst_cycles = 5;
    write_FPR((uint32_t)read_uint32((uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value)), INST_FT(value), cpu);
    advance_PC(cpu);
}

void SDC1(uint32_t value, cpu_t* cpu)
{
    write_uint64(read_FPR(INST_FT(value), cpu), (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value));
    advance_PC(cpu);
}

void SWC1(uint32_t value, cpu_t* cpu)
{
    write_uint32(read_FPR(INST_FT(value), cpu), read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value));
    advance_PC(cpu);
}

void ADDI(uint32_t value, cpu_t* cpu)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    ADD_imm(INST_RS(value), INST_IMM(value), INST_RT(value), cpu);
    advance_PC(cpu);
}

void ADDIU(uint32_t value, cpu_t* cpu)
{
    ADD_imm(INST_RS(value), INST_IMM(value), INST_RT(value), cpu);
    advance_PC(cpu);
}

void DADDI(uint32_t value, cpu_t* cpu)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    DADD_imm(INST_RS(value), INST_IMM(value), INST_RT(value), cpu);
    advance_PC(cpu);
}

void DADDIU(uint32_t value, cpu_t* cpu)
{
    DADD_imm(INST_RS(value), INST_IMM(value), INST_RT(value), cpu);
    advance_PC(cpu);
}

void ANDI(uint32_t value, cpu_t* cpu)
{
    AND_imm(INST_RS(value), INST_IMM(value), INST_RT(value), cpu);
    advance_PC(cpu);
}

void ORI(uint32_t value, cpu_t* cpu)
{
    OR_imm(INST_RS(value), INST_IMM(value), INST_RT(value), cpu);
    advance_PC(cpu);
}

void XORI(uint32_t value, cpu_t* cpu)
{
    XOR_imm(INST_RS(value), INST_IMM(value), INST_RT(value), cpu);
    advance_PC(cpu);
}

void LB(uint32_t value, cpu_t* cpu)
{
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value);
    cpu->curr_inst_cycles = 5;
    if (cpu->rsp)
    {
        if (addr > 0xFFF) write_GPR(0, INST_RT(value), cpu);
        else write_GPR((char)((char*)SP_DMEM_RW)[addr & 0xFFF], INST_RT(value), cpu);
    }
    else
    {
        write_GPR((char)read_uint8(addr), INST_RT(value), cpu);
    }
    advance_PC(cpu);
}

void LBU(uint32_t value, cpu_t* cpu)
{
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value);
    cpu->curr_inst_cycles = 5;
    if (cpu->rsp)
    {
        if (addr > 0xFFF) write_GPR(0, INST_RT(value), cpu);
        else write_GPR((uint8_t)((uint8_t*)SP_DMEM_RW)[addr & 0xFFF], INST_RT(value), cpu);
    }
    else
    {
        write_GPR((uint8_t)read_uint8(addr), INST_RT(value), cpu);
    }
    advance_PC(cpu);
}

void LD(uint32_t value, cpu_t* cpu)
{
    cpu->curr_inst_cycles = 5;
    write_GPR(read_uint64((uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value)), INST_RT(value), cpu);
    advance_PC(cpu);
}

void LDL(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void LDR(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void LH(uint32_t value, cpu_t* cpu)
{
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value);
    cpu->curr_inst_cycles = 5;
    if (cpu->rsp)
    {
        if (addr > 0xFFF) write_GPR(0, INST_RT(value), cpu);
        else write_GPR((short)bswap_16(*(short*)&(((uint8_t*)SP_DMEM_RW)[addr & 0xFFF])), INST_RT(value), cpu);
    }
    else
    {
        write_GPR((short)read_uint16(addr), INST_RT(value), cpu);
    }
    advance_PC(cpu);
}

void LHU(uint32_t value, cpu_t* cpu)
{
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value);
    cpu->curr_inst_cycles = 5;
    if (cpu->rsp)
    {
        if (addr > 0xFFF) write_GPR(0, INST_RT(value), cpu);
        else write_GPR((uint16_t)bswap_16(*(uint16_t*)&(((uint8_t*)SP_DMEM_RW)[addr & 0xFFF])), INST_RT(value), cpu);
    }
    else
    {
        write_GPR((uint16_t)read_uint16(addr), INST_RT(value), cpu);
    }
    advance_PC(cpu);
}

void LL(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void LLD(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void LUI(uint32_t value, cpu_t* cpu)
{
    write_GPR((uint32_t)INST_IMM(value) << 16, INST_RT(value), cpu);
    advance_PC(cpu);
}

void LW(uint32_t value, cpu_t* cpu)
{
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value);
    cpu->curr_inst_cycles = 5;
    if (cpu->rsp)
    {
        if (addr > 0xFFF) write_GPR(0, INST_RT(value), cpu);
        else write_GPR((uint32_t)bswap_32(*(uint32_t*)&(((uint8_t*)SP_DMEM_RW)[addr & 0xFFF])), INST_RT(value), cpu);
    }
    else
    {
        write_GPR((uint32_t)read_uint32(addr), INST_RT(value), cpu);
    }
    advance_PC(cpu);
}

void LWL(uint32_t value, cpu_t* cpu)
{
    cpu->curr_inst_cycles = 5;
    short    offs      = (short)INST_IMM(value);
    uint32_t base_addr = (uint32_t)read_GPR(INST_RS(value), cpu);
    uint32_t addr      = (base_addr & ~0x3) + offs;
    uint32_t mem       = (uint32_t)read_uint32(addr);

    uint16_t shift = (offs % 8) * 8;
    uint64_t reg   = read_GPR(INST_RT(value), cpu);
    uint64_t res   = ((int)mem) & 0xFFFFFFFF00000000;
    res           |= ((mem << shift) | (reg & ((uint64_t)0xFFFFFFFF >> (32 - shift)))) & 0xFFFFFFFF;
    write_GPR(res, INST_RT(value), cpu);

    advance_PC(cpu);
}

void LWR(uint32_t value, cpu_t* cpu)
{
    cpu->curr_inst_cycles = 5;
    short    offs      = (short)INST_IMM(value);
    uint32_t base_addr = (uint32_t)read_GPR(INST_RS(value), cpu);
    uint32_t addr      = (base_addr & ~0x3) + offs;
    uint32_t mem       = (uint32_t)read_uint32(addr);

    uint16_t shift = (offs % 8) * 8;
    uint64_t reg   = read_GPR(INST_RT(value), cpu);
    uint64_t res   = 0xFFFFFFFF00000000;
    res           |= ((mem >> (24 - shift)) | (reg & ((uint64_t)0xFFFFFFFF << (shift + 8)))) & 0xFFFFFFFF;
    write_GPR(res, INST_RT(value), cpu);

    advance_PC(cpu);
}

void LWU(uint32_t value, cpu_t* cpu)
{
    cpu->curr_inst_cycles = 5;
    write_GPR((uint32_t)read_uint32((uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value)), INST_RT(value), cpu);
    advance_PC(cpu);
}

void SB(uint32_t value, cpu_t* cpu)
{
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value);
    if (cpu->rsp)
    {
        if (addr > 0xFFF)
        {
            advance_PC(cpu);
            return;
        }
        ((uint8_t*)SP_DMEM_RW)[addr & 0xFFF] = (uint8_t)read_GPR(INST_RT(value), cpu);
    }
    else
    {
        write_uint8(read_GPR(INST_RT(value), cpu), addr);
    }
    advance_PC(cpu);
}

void SC(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void SCD(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void SD(uint32_t value, cpu_t* cpu)
{
    write_uint64(read_GPR(INST_RT(value), cpu), (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value));
    advance_PC(cpu);
}

void SDL(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void SDR(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void SH(uint32_t value, cpu_t* cpu)
{
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value);
    if (cpu->rsp)
    {
        if (addr > 0xFFF)
        {
            advance_PC(cpu);
            return;
        }
        *((uint16_t*)&(((uint8_t*)SP_DMEM_RW)[addr & 0xFFF])) = bswap_16((uint16_t)read_GPR(INST_RT(value), cpu));
    }
    else
    {
        write_uint16(read_GPR(INST_RT(value), cpu), addr);
    }
    advance_PC(cpu);
}

void SLTI(uint32_t value, cpu_t* cpu)
{
    SET_cond(INST_RT(value), (int)read_GPR(INST_RS(value), cpu) < (short)INST_IMM(value), cpu);
    advance_PC(cpu);
}

void SLTIU(uint32_t value, cpu_t* cpu)
{
    SET_cond(INST_RT(value), (uint32_t)read_GPR(INST_RS(value), cpu) < (uint16_t)INST_IMM(value), cpu);
    advance_PC(cpu);
}

void SW(uint32_t value, cpu_t* cpu)
{
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value), cpu) + (short)INST_IMM(value);
    if (cpu->rsp)
    {
        if (addr > 0xFFF)
        {
            advance_PC(cpu);
            return;
        }
        *((uint32_t*)&(((uint8_t*)SP_DMEM_RW)[addr & 0xFFF])) = bswap_32((uint32_t)read_GPR(INST_RT(value), cpu));
    }
    else
    {
        write_uint32(read_GPR(INST_RT(value), cpu), addr);
    }
    advance_PC(cpu);
}

void SWL(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void SWR(uint32_t value, cpu_t* cpu)
{
    MIPS_undefined_inst_error(value, cpu);
}

void BEQ(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond(INST_IMM(value), (uint32_t)read_GPR(INST_RS(value), cpu) == (uint32_t)read_GPR(INST_RT(value), cpu), cpu);
    advance_PC(cpu);
}

void BEQL(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond_likely(INST_IMM(value), (uint32_t)read_GPR(INST_RS(value), cpu) == (uint32_t)read_GPR(INST_RT(value), cpu), cpu);
    advance_PC(cpu);
}

void BGTZ(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) > 0, cpu);
    advance_PC(cpu);
}

void BGTZL(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) > 0, cpu);
    advance_PC(cpu);
}

void BLEZ(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) <= 0, cpu);
    advance_PC(cpu);
}

void BLEZL(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value), cpu) <= 0, cpu);
    advance_PC(cpu);
}

void BNE(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond(INST_IMM(value), (uint32_t)read_GPR(INST_RS(value), cpu) != (uint32_t)read_GPR(INST_RT(value), cpu), cpu);
    advance_PC(cpu);
}

void BNEL(uint32_t value, cpu_t* cpu)
{
    BRANCH_cond_likely(INST_IMM(value), (uint32_t)read_GPR(INST_RS(value), cpu) != (uint32_t)read_GPR(INST_RT(value), cpu), cpu);
    advance_PC(cpu);
}

void J(uint32_t value, cpu_t* cpu)
{
    JUMP_imm(INST_TARGET(value), cpu);
    advance_PC(cpu);
}

void JAL(uint32_t value, cpu_t* cpu)
{
    link_PC(cpu);
    JUMP_imm(INST_TARGET(value), cpu);
    advance_PC(cpu);
}

void CACHE(uint32_t value, cpu_t* cpu)
{
    advance_PC(cpu); // Does nothing (we don't need to emulate this)
}
