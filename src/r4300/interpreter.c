#include "r4300/interpreter.h"

#include "common.h"

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

#define INST_FMT(x) INST_RS(x)
#define INST_FT(x) INST_RT(x)
#define INST_FS(x) INST_RD(x)
#define INST_FD(x) INST_SA(x)

bool is_branching = false;

uint32_t curr_target = 0;

uint8_t  curr_inst_cycles = 0;
uint64_t cycles           = 0;
uint64_t all_cycles       = 0;
uint32_t vi_cycle_count   = 0;

__attribute__((__always_inline__)) static inline void undefined_inst_error(uint32_t value)
{
    fprintf(stderr, "ERROR: Unimplemented Instruction 0x%x!  PC: 0x%x\n", value, (uint32_t)regs.PC.value);
    is_running = false;
}

__attribute__((__always_inline__)) static inline void ADD_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR((long)((uint32_t)read_GPR(reg1) + (uint32_t)read_GPR(reg2)), dst);
}

__attribute__((__always_inline__)) static inline void ADD_imm(uint8_t reg, uint16_t imm, uint8_t dst)
{
    write_GPR((long)((uint32_t)read_GPR(reg) + (short)imm), dst);
}

__attribute__((__always_inline__)) static inline void SUB_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR((long)((uint32_t)read_GPR(reg1) - (uint32_t)read_GPR(reg2)), dst);
}

__attribute__((__always_inline__)) static inline void DIV_reg(uint8_t reg1, uint8_t reg2)
{
    if ((int)read_GPR(reg2))
    {
        write_LO((int)read_GPR(reg1) / (long)((int)read_GPR(reg2)));
        write_HI((int)read_GPR(reg1) % (long)((int)read_GPR(reg2)));
    }
    else
    {
        if ((int)read_GPR(reg1) > 0)
            write_LO(0xFFFFFFFF);
        else if ((int)read_GPR(reg1) < 0)
            write_LO(0x00000001);

        write_HI((int)read_GPR(reg1));
    }
}

__attribute__((__always_inline__)) static inline void DIVU_reg(uint8_t reg1, uint8_t reg2)
{
    if ((uint32_t)read_GPR(reg2))
    {
        write_LO((uint32_t)read_GPR(reg1) / (uint64_t)((uint32_t)read_GPR(reg2)));
        write_HI((uint32_t)read_GPR(reg1) % (uint64_t)((uint32_t)read_GPR(reg2)));
    }
    else
    {
        write_LO(0xFFFFFFFF);
        write_HI((uint32_t)read_GPR(reg1));
    }
}

__attribute__((__always_inline__)) static inline void MULT_reg(uint8_t reg1, uint8_t reg2)
{
    long res = (int)read_GPR(reg1) * (long)((int)read_GPR(reg2));
    write_LO(res & 0xFFFFFFFF);
    write_HI(res >> 32);
}

__attribute__((__always_inline__)) static inline void MULTU_reg(uint8_t reg1, uint8_t reg2)
{
    uint64_t res = (uint32_t)read_GPR(reg1) * (uint64_t)((uint32_t)read_GPR(reg2));
    write_LO(res & 0xFFFFFFFF);
    write_HI((long)res >> 32);
}

__attribute__((__always_inline__)) static inline void SLL_imm(uint8_t reg, uint8_t dst, uint8_t sa)
{
    write_GPR(read_GPR(reg) << sa, dst);
}

__attribute__((__always_inline__)) static inline void SLL_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR((uint32_t)read_GPR(reg1) << (read_GPR(reg2) & 0x1F), dst);
}

__attribute__((__always_inline__)) static inline void SRA_imm(uint8_t reg, uint8_t dst, uint8_t sa)
{
    write_GPR((long)((int)read_GPR(reg) >> sa), dst);
}

__attribute__((__always_inline__)) static inline void SRA_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR((long)((int)read_GPR(reg1) >> (read_GPR(reg2) & 0x1F)), dst);
}

__attribute__((__always_inline__)) static inline void SRL_imm(uint8_t reg, uint8_t dst, uint8_t sa)
{
    write_GPR((uint32_t)read_GPR(reg) >> sa, dst);
}

__attribute__((__always_inline__)) static inline void SRL_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR(read_GPR(reg1) >> (read_GPR(reg2) & 0x1F), dst);
}

__attribute__((__always_inline__)) static inline void DADD_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR(read_GPR(reg1) + read_GPR(reg2), dst);
}

__attribute__((__always_inline__)) static inline void DADD_imm(uint8_t reg1, uint16_t imm, uint8_t dst)
{
    write_GPR(read_GPR(reg1) + (short)imm, dst);
}

__attribute__((__always_inline__)) static inline void DSUB_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR(read_GPR(reg1) - read_GPR(reg2), dst);
}

__attribute__((__always_inline__)) static inline void DDIV_reg(uint8_t reg1, uint8_t reg2)
{
    if ((long)read_GPR(reg2))
    {
        write_LO((long)read_GPR(reg1) / (__int128_t)((long)read_GPR(reg2)));
        write_HI((long)read_GPR(reg1) % (__int128_t)((long)read_GPR(reg2)));
    }
    else
    {
        if ((long)read_GPR(reg1) > 0)
            write_LO(0xFFFFFFFFFFFFFFFF);
        else if ((long)read_GPR(reg1) < 0)
            write_LO(0x0000000000000001);

        write_HI((long)read_GPR(reg1));
    }
}

__attribute__((__always_inline__)) static inline void DDIVU_reg(uint8_t reg1, uint8_t reg2)
{
    if ((uint64_t)read_GPR(reg2))
    {
        write_LO((uint64_t)read_GPR(reg1) / (__uint128_t)((uint64_t)read_GPR(reg2)));
        write_HI((uint64_t)read_GPR(reg1) % (__uint128_t)((uint64_t)read_GPR(reg2)));
    }
    else
    {
        write_LO(0xFFFFFFFFFFFFFFFF);
        write_HI((uint64_t)read_GPR(reg1));
    }
}

__attribute__((__always_inline__)) static inline void DMULT_reg(uint8_t reg1, uint8_t reg2)
{
    __int128_t res = (long)read_GPR(reg1) * (__int128_t)((long)read_GPR(reg2));
    write_LO(res);
    write_HI(res >> 64);
}

__attribute__((__always_inline__)) static inline void DMULTU_reg(uint8_t reg1, uint8_t reg2)
{
    if ((uint64_t)read_GPR(reg2) == 0xFFFFFFFF)
    {
        write_LO(0x0000000000000001);
        write_HI(0xFFFFFFFFFFFFFFFE);
    }
    else if ((uint64_t)read_GPR(reg1) == 0xFFFFFFFF)
    {
        write_LO(0xFFFFFFFFFFFFFFFE);
        write_HI(0x0000000000000001);
    }
    else
    {
        __uint128_t res = (uint64_t)read_GPR(reg1) * (__uint128_t)((uint64_t)read_GPR(reg2));
        write_LO(res);
        write_HI((__int128_t)res >> 64);
    }
}

__attribute__((__always_inline__)) static inline void DSLL_imm(uint8_t reg, uint8_t dst, uint8_t sa)
{
    write_GPR(read_GPR(reg) << sa, dst);
}

__attribute__((__always_inline__)) static inline void DSLL_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR(read_GPR(reg1) << (read_GPR(reg2) & 0x3F), dst);
}

__attribute__((__always_inline__)) static inline void DSRA_imm(uint8_t reg, uint8_t dst, uint8_t sa)
{
    write_GPR((long)read_GPR(reg) >> sa, dst);
}

__attribute__((__always_inline__)) static inline void DSRA_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR((long)read_GPR(reg1) >> (read_GPR(reg2) & 0x3F), dst);
}

__attribute__((__always_inline__)) static inline void DSRL_imm(uint8_t reg, uint8_t dst, uint8_t sa)
{
    write_GPR(read_GPR(reg) >> sa, dst);
}

__attribute__((__always_inline__)) static inline void DSRL_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR(read_GPR(reg1) >> (read_GPR(reg2) & 0x3F), dst);
}

__attribute__((__always_inline__)) static inline void AND_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR((long)((uint32_t)read_GPR(reg1) & (uint32_t)read_GPR(reg2)), dst);
}

__attribute__((__always_inline__)) static inline void AND_imm(uint8_t reg, uint16_t imm, uint8_t dst)
{
    write_GPR((long)((uint32_t)read_GPR(reg) & imm), dst);
}

__attribute__((__always_inline__)) static inline void OR_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR((long)((uint32_t)read_GPR(reg1) | (uint32_t)read_GPR(reg2)), dst);
}

__attribute__((__always_inline__)) static inline void OR_imm(uint8_t reg, uint16_t imm, uint8_t dst)
{
    write_GPR((long)((uint32_t)read_GPR(reg) | imm), dst);
}

__attribute__((__always_inline__)) static inline void XOR_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR((long)((uint32_t)read_GPR(reg1) ^ (uint32_t)read_GPR(reg2)), dst);
}

__attribute__((__always_inline__)) static inline void XOR_imm(uint8_t reg, uint16_t imm, uint8_t dst)
{
    write_GPR((long)((uint32_t)read_GPR(reg) ^ imm), dst);
}

__attribute__((__always_inline__)) static inline void NOR_reg(uint8_t reg1, uint8_t reg2, uint8_t dst)
{
    write_GPR((long)(~((uint32_t)read_GPR(reg1) | (uint32_t)read_GPR(reg2))), dst);
}

__attribute__((__always_inline__)) static inline void SET_cond(uint8_t dst, bool Cond)
{
    write_GPR((uint32_t)Cond, dst);
}

__attribute__((__always_inline__)) static inline void BRANCH_cond(uint16_t imm, bool Cond)
{
    is_branching = Cond;
    if (Cond) curr_target = ((uint32_t)regs.PC.value + 4) + (int)(((short)imm) << 2);
}

__attribute__((__always_inline__)) static inline void BRANCH_cond_likely(uint16_t imm, bool Cond)
{
    BRANCH_cond(imm, Cond);
    if (!Cond) advance_PC();
}

__attribute__((__always_inline__)) static inline void TRAP_cond(bool Cond)
{
    if (Cond) invoke_trap();
}

__attribute__((__always_inline__)) static inline void JUMP_reg(uint8_t reg)
{
    is_branching = true;
    curr_target  = read_GPR(reg);
}

__attribute__((__always_inline__)) static inline void JUMP_imm(uint32_t target)
{
    is_branching = true;
    curr_target  = (target << 2) | (((uint32_t)regs.PC.value + 4) & 0xF0000000);
}

__attribute__((__always_inline__)) static inline void link_PC(void)
{
    regs.GPR[31].value = (uint32_t)regs.PC.value + 8; // Hyah!
}

uint32_t current_scanline = 0;

void interp_step(void)
{
    bool should_branch = is_branching;

    uint32_t inst = bswap_32(*(uint32_t*)get_real_memory_loc((uint32_t)regs.PC.value));
    opcode_t op   = opcode_table[INST_OP(inst)];

    if (op.interpret == NULL) 
    {
        undefined_inst_error(inst);
        return;
    }

    regs.GPR[0].value = 0;

    if ((uint32_t)regs.COP0[COP0_COUNT].value >= 0xFFFFFFFF)
    {
        regs.COP0[COP0_COUNT].value = 0;

        cycles = 0;
    }

    if (vi_cycle_count >= (uint32_t)((CPU_mhz * 1000000) / config.refresh_rate))
    {
        if (current_scanline >= (bswap_32(VI_V_SYNC_REG_RW) & 0x3FF)) current_scanline = 0;
        else ++current_scanline;
        if (current_scanline == bswap_32(VI_INTR_REG_RW))
            invoke_mi_interrupt(MI_INTR_VI);
        VI_CURRENT_REG_R = bswap_32(current_scanline);

        vi_cycle_count = 0;
    }

    curr_inst_cycles = 1;
    op.interpret(inst);

    cycles         += curr_inst_cycles;
    all_cycles     += curr_inst_cycles;
    vi_cycle_count += curr_inst_cycles;

    regs.COP0[COP0_COUNT].value = (uint32_t)(cycles >> 1);
    if (regs.COP0[COP0_RANDOM].value < regs.COP0[COP0_WIRED].value 
     || regs.COP0[COP0_RANDOM].value == 0) regs.COP0[COP0_RANDOM].value = 0x1F;
    --regs.COP0[COP0_RANDOM].value;

    if (!is_running) return;

    if (should_branch) // If we should branch (aka the last instruction was a branch instruction).
    {
        regs.PC.value = curr_target; // Then set to the PC the target.
        is_branching = false; // And reset the variables.
        curr_target = 0;
    }

    poll_int();
}

__attribute__((__always_inline__)) static inline void ADD(uint32_t value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    ADD_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void ADDU(uint32_t value)
{
    ADD_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void AND(uint32_t value)
{
    AND_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void BREAK(uint32_t value)
{
    invoke_break();
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DADD(uint32_t value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    DADD_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DADDU(uint32_t value)
{
    DADD_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DDIV(uint32_t value)
{
    DDIV_reg(INST_RS(value), INST_RT(value));
    curr_inst_cycles = 69;
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DDIVU(uint32_t value)
{
    DDIVU_reg(INST_RS(value), INST_RT(value));
    curr_inst_cycles = 69;
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DIV(uint32_t value)
{
    DIV_reg(INST_RS(value), INST_RT(value));
    curr_inst_cycles = 37;
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DIVU(uint32_t value)
{
    DIVU_reg(INST_RS(value), INST_RT(value));
    curr_inst_cycles = 37;
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DMULT(uint32_t value)
{
    DMULT_reg(INST_RS(value), INST_RT(value));
    curr_inst_cycles = 8;
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DMULTU(uint32_t value)
{
    DMULTU_reg(INST_RS(value), INST_RT(value));
    curr_inst_cycles = 8;
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSLL(uint32_t value)
{
    DSLL_imm(INST_RT(value), INST_RD(value), INST_SA(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSLLV(uint32_t value)
{
    DSLL_reg(INST_RT(value), INST_RS(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSLL32(uint32_t value)
{
    DSLL_imm(INST_RT(value), INST_RD(value), INST_SA(value) + 32);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSRA(uint32_t value)
{
    DSRA_imm(INST_RT(value), INST_RD(value), INST_SA(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSRAV(uint32_t value)
{
    DSRA_reg(INST_RT(value), INST_RS(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSRA32(uint32_t value)
{
    DSRA_imm(INST_RT(value), INST_RD(value), INST_SA(value) + 32);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSRL(uint32_t value)
{
    DSRL_imm(INST_RT(value), INST_RD(value), INST_SA(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSRLV(uint32_t value)
{
    DSRL_reg(INST_RT(value), INST_RS(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSRL32(uint32_t value)
{
    DSRL_imm(INST_RT(value), INST_RD(value), INST_SA(value) + 32);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSUB(uint32_t value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    DSUB_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DSUBU(uint32_t value)
{
    DSUB_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void JALR(uint32_t value)
{
    link_PC();
    JUMP_reg(INST_RS(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void JR(uint32_t value)
{
    JUMP_reg(INST_RS(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void MFHI(uint32_t value)
{
    write_GPR(read_HI(), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void MFLO(uint32_t value)
{
    write_GPR(read_LO(), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void MTHI(uint32_t value)
{
    write_HI(read_GPR(INST_RS(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void MTLO(uint32_t value)
{
    write_LO(read_GPR(INST_RS(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void MULT(uint32_t value)
{
    MULT_reg(INST_RS(value), INST_RT(value));
    curr_inst_cycles = 5;
    advance_PC();
}

__attribute__((__always_inline__)) static inline void MULTU(uint32_t value)
{
    MULTU_reg(INST_RS(value), INST_RT(value));
    curr_inst_cycles = 5;
    advance_PC();
}

__attribute__((__always_inline__)) static inline void NOR(uint32_t value)
{
    NOR_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void OR(uint32_t value)
{
    OR_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SLL(uint32_t value)
{
    SLL_imm(INST_RT(value), INST_RD(value), INST_SA(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SLLV(uint32_t value)
{
    SLL_reg(INST_RT(value), INST_RS(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SLT(uint32_t value)
{
    SET_cond(INST_RD(value), (long)read_GPR(INST_RS(value)) < (long)read_GPR(INST_RT(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SLTU(uint32_t value)
{
    SET_cond(INST_RD(value), read_GPR(INST_RS(value)) < read_GPR(INST_RT(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SRA(uint32_t value)
{
    SRA_imm(INST_RT(value), INST_RD(value), INST_SA(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SRAV(uint32_t value)
{
    SRA_reg(INST_RT(value), INST_RS(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SRL(uint32_t value)
{
    SRL_imm(INST_RT(value), INST_RD(value), INST_SA(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SRLV(uint32_t value)
{
    SRL_reg(INST_RT(value), INST_RS(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SUB(uint32_t value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    SUB_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void SUBU(uint32_t value)
{
    SUB_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TEQ(uint32_t value)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value)) == (uint32_t)read_GPR(INST_RT(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TGE(uint32_t value)
{
    TRAP_cond((int)read_GPR(INST_RS(value)) >= (int)read_GPR(INST_RT(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TGEU(uint32_t value)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value)) >= (uint32_t)read_GPR(INST_RT(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TLT(uint32_t value)
{
    TRAP_cond((int)read_GPR(INST_RS(value)) < (int)read_GPR(INST_RT(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TLTU(uint32_t value)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value)) < (uint32_t)read_GPR(INST_RT(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TNE(uint32_t value)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value)) != (uint32_t)read_GPR(INST_RT(value)));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void XOR(uint32_t value)
{
    XOR_reg(INST_RS(value), INST_RT(value), INST_RD(value));
    advance_PC();
}

void SPECIAL(uint32_t value)
{
    switch (INST_FUNCT(value))
    {
        case 0b100000: // ADD
            ADD(value);
            return;
        case 0b100001: // ADDU
            ADDU(value);
            return;
        case 0b100100: // AND
            AND(value);
            return;
        case 0b001101: // BREAK
            BREAK(value);
            return;
        case 0b101100: // DADD
            DADD(value);
            return;
        case 0b101101: // DADDU
            DADDU(value);
            return;
        case 0b011110: // DDIV
            DDIV(value);
            return;
        case 0b011111: // DDIVU
            DDIVU(value);
            return;
        case 0b011010: // DIV
            DIV(value);
            return;
        case 0b011011: // DIVU
            DIVU(value);
            return;
        case 0b011100: // DMULT
            DMULT(value);
            return;
        case 0b011101: // DMULT
            DMULTU(value);
            return;
        case 0b111000: // DSLL
            DSLL(value);
            return;
        case 0b010100: // DSLLV
            DSLLV(value);
            return;
        case 0b111100: // DSLL32
            DSLL32(value);
            return;
        case 0b111011: // DSRA
            DSRA(value);
            return;
        case 0b010111: // DSRAV
            DSRAV(value);
            return;
        case 0b111111: // DSRA32
            DSRA32(value);
            return;
        case 0b111010: // DSRL
            DSRL(value);
            return;
        case 0b010110: // DSRLV
            DSRLV(value);
            return;
        case 0b111110: // DSRL32
            DSRL32(value);
            return;
        case 0b101110: // DSUB
            DSUB(value);
            return;
        case 0b101111: // DSUBU
            DSUBU(value);
            return;
        case 0b001001: // JALR
            JALR(value);
            return;
        case 0b001000: // JR
            JR(value);
            return;
        case 0b010000: // MFHI
            MFHI(value);
            return;
        case 0b010010: // MFLO
            MFLO(value);
            return;
        case 0b010001: // MTHI
            MTHI(value);
            return;
        case 0b010011: // MTLO
            MTLO(value);
            return;
        case 0b011000: // MULT
            MULT(value);
            return;
        case 0b011001: // MULTU
            MULTU(value);
            return;
        case 0b100111: // NOR
            NOR(value);
            return;
        case 0b100101: // OR
            OR(value);
            return;
        case 0b000000: // SLL
            SLL(value);
            return;
        case 0b000100: // SLLV
            SLLV(value);
            return;
        case 0b101010: // SLT
            SLT(value);
            return;
        case 0b101011: // SLTU
            SLTU(value);
            return;
        case 0b000011: // SRA
            SRA(value);
            return;
        case 0b000111: // SRAV
            SRAV(value);
            return;
        case 0b000010: // SRL
            SRL(value);
            return;
        case 0b000110: // SRLV
            SRLV(value);
            return;
        case 0b100010: // SUB
            SUB(value);
            return;
        case 0b100011: // SUBU
            SUBU(value);
            return;
        case 0b001111: // SYNC
            return; // Executes as a NOP on the VR4300.
        case 0b110100: // TEQ
            TEQ(value);
            return;
        case 0b110000: // TGE
            TGE(value);
            return;
        case 0b110001: // TGEU
            TGEU(value);
            return;
        case 0b110010: // TLT
            TLT(value);
            return;
        case 0b110011: // TLTU
            TLTU(value);
            return;
        case 0b110110: // TNE
            TNE(value);
            return;
        case 0b100110: // XOR
            XOR(value);
            return;
    }
    undefined_inst_error(value);
}

__attribute__((__always_inline__)) static inline void BGEZ(uint32_t value)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value)) >= 0);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void BGEZAL(uint32_t value)
{
    link_PC();
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value)) >= 0);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void BGEZALL(uint32_t value)
{
    link_PC();
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value)) >= 0);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void BGEZL(uint32_t value)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value)) >= 0);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void BLTZ(uint32_t value)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value)) < 0);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void BLTZAL(uint32_t value)
{
    link_PC();
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value)) < 0);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void BLTZALL(uint32_t value)
{
    link_PC();
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value)) < 0);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void BLTZL(uint32_t value)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value)) < 0);
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TEQI(uint32_t value)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value)) == (int)INST_IMM(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TGEI(uint32_t value)
{
    TRAP_cond((int)read_GPR(INST_RS(value)) >= (int)INST_IMM(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TGEIU(uint32_t value)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value)) >= (uint32_t)INST_IMM(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TLTI(uint32_t value)
{
    TRAP_cond((int)read_GPR(INST_RS(value)) < (int)INST_IMM(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TLTIU(uint32_t value)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value)) < (uint32_t)INST_IMM(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TNEI(uint32_t value)
{
    TRAP_cond((uint32_t)read_GPR(INST_RS(value)) != (int)INST_IMM(value));
    advance_PC();
}

void REGIMM(uint32_t value)
{
    switch (INST_RT(value))
    {
        case 0b00001: // BGEZ
            BGEZ(value);
            return;
        case 0b10001: // BGEZAL
            BGEZAL(value);
            return;
        case 0b10011: // BGEZALL
            BGEZALL(value);
            return;
        case 0b00011: // BGEZL
            BGEZL(value);
            return;
        case 0b00000: // BLTZ
            BLTZ(value);
            return;
        case 0b10000: // BLTZAL
            BLTZAL(value);
            return;
        case 0b10010: // BLTZALL
            BLTZALL(value);
            return;
        case 0b00010: // BLTZL
            BLTZL(value);
            return;
        case 0b01100: // TEQI
            TEQI(value);
            return;
        case 0b01000: // TGEI
            TGEI(value);
            return;
        case 0b01001: // TGEIU
            TGEIU(value);
            return;
        case 0b01010: // TLTI
            TLTI(value);
            return;
        case 0b01011: // TLTIU
            TLTIU(value);
            return;
        case 0b01110: // TNEI
            TNEI(value);
            return;
    }
    undefined_inst_error(value);
}

__attribute__((__always_inline__)) static inline void DMFC0(uint32_t value)
{
    write_GPR(read_COP0(INST_RD(value)), INST_RT(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DMTC0(uint32_t value)
{
    write_COP0(read_GPR(INST_RT(value)), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void ERET(uint32_t value)
{
    uint32_t NewPC = (regs.COP0[COP0_STATUS].value & 0b100) > 0 
                    ? (uint32_t)regs.COP0[COP0_ERROREPC].value 
                    : (uint32_t)regs.COP0[COP0_EPC].value;
    regs.PC.value = NewPC;
    regs.COP0[COP0_STATUS].value &= ~0b100;
    regs.LLbit = false;
}

__attribute__((__always_inline__)) static inline void MFC0(uint32_t value)
{
    write_GPR((uint32_t)read_COP0(INST_RD(value)), INST_RT(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void MTC0(uint32_t value)
{
    write_COP0((uint32_t)read_GPR(INST_RT(value)), INST_RD(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TLBP(uint32_t value)
{
    probe_TLB();
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TLBR(uint32_t value)
{
    read_TLB_entry();
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TLBWI(uint32_t value)
{
    write_TLB_entry_indexed();
    advance_PC();
}

__attribute__((__always_inline__)) static inline void TLBWR(uint32_t value)
{
    write_TLB_entry_random();
    advance_PC();
}

void COP0(uint32_t value)
{
    switch (INST_RS(value))
    {
        case 0b00001: // DMFC0
            DMFC0(value);
            return;
        case 0b00101: // DMTC0
            DMTC0(value);
            return;
        case 0b10000: // CO
            switch (INST_FUNCT(value))
            {
                case 0b011000: // ERET
                    ERET(value);
                    return;
                case 0b001000: // TLBP
                    TLBP(value);
                    return;
                case 0b000001: // TLBR
                    TLBR(value);
                    return;
                case 0b000010: // TLBWI
                    TLBWI(value);
                    return;
                case 0b000110: // TLBWR
                    TLBWR(value);
                    return;
            }
            break;
        case 0b00000: // MFC0
            MFC0(value);
            return;
        case 0b00100: // MTC0
            MTC0(value);
            return;
    }
    undefined_inst_error(value);
}

__attribute__((__always_inline__)) static inline void ABS_fmt(uint32_t value)
{
    uint64_t fs = read_FPR(INST_FS(value));
    uint64_t fd = 0;
    if (INST_FMT(value) == 16) // Float
    {
        float float_fs = *(float*)&fs;

        if (isnan(float_fs))
        {
            // Invalid Operation
        }

        float_fs = fabsf(float_fs);
        fd = *(uint32_t*)&float_fs;
    }
    else if (INST_FMT(value) == 17) // Double
    {
        double double_fs = *(double*)&fs;

        if (isnan(double_fs))
        {
            // Invalid Operation
        }

        double_fs = fabsf(double_fs);
        fd = *(uint64_t*)&double_fs;
    }
    else
        undefined_inst_error(value);
    
    write_FPR(fd, INST_FD(value));

    advance_PC();
}

__attribute__((__always_inline__)) static inline void CFC1(uint32_t value)
{
    if (INST_RD(value) == 31 || INST_RD(value) == 0)
    {
        write_GPR((uint32_t)read_FPR(INST_RD(value)), INST_RT(value));
    }
    else
    {
        undefined_inst_error(value);
    }
    advance_PC();
}

__attribute__((__always_inline__)) static inline void CTC1(uint32_t value)
{
    if (INST_RD(value) == 31 || INST_RD(value) == 0)
    {
        write_FPR((uint32_t)read_GPR(INST_RT(value)), INST_RD(value));
    }
    else
    {
        undefined_inst_error(value);
    }
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DMFC1(uint32_t value)
{
    write_GPR(read_FPR(INST_FS(value)), INST_RT(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void DMTC1(uint32_t value)
{
    write_FPR(read_GPR(INST_RT(value)), INST_FS(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void MFC1(uint32_t value)
{
    write_GPR((uint32_t)read_FPR(INST_FS(value)), INST_RT(value));
    advance_PC();
}

__attribute__((__always_inline__)) static inline void MTC1(uint32_t value)
{
    write_FPR((uint32_t)read_GPR(INST_RT(value)), INST_FS(value));
    advance_PC();
}

void COP1(uint32_t value)
{
    if (INST_FUNCT(value) == 0 && INST_SA(value) == 0)
    {
        switch (INST_RS(value))
        {
            case 0b00010: // CFC1
                CFC1(value);
                return;
            case 0b00110: // CTC1
                CTC1(value);
                return;
            case 0b00001: // DMFC1
                DMFC1(value);
                return;
            case 0b00101: // DMTC1
                DMTC1(value);
                return;
            case 0b00000: // MFC1
                MFC1(value);
                return;
            case 0b00100: // MTC1
                MTC1(value);
                return;
        }
    }
    else if (INST_FUNCT(value) != 0)
    {
        switch (INST_FUNCT(value))
        {
            case 0b000101: // ABS.fmt
                ABS_fmt(value);
                return;
        }
    }
    
    undefined_inst_error(value);
}

void ADDI(uint32_t value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    ADD_imm(INST_RS(value), INST_IMM(value), INST_RT(value));
    advance_PC();
}

void ADDIU(uint32_t value)
{
    ADD_imm(INST_RS(value), INST_IMM(value), INST_RT(value));
    advance_PC();
}

void DADDI(uint32_t value)
{
    // TODO: Correctly check for Overflow and Underflow and throw the exceptions accordingly.
    DADD_imm(INST_RS(value), INST_IMM(value), INST_RT(value));
    advance_PC();
}

void DADDIU(uint32_t value)
{
    DADD_imm(INST_RS(value), INST_IMM(value), INST_RT(value));
    advance_PC();
}

void ANDI(uint32_t value)
{
    AND_imm(INST_RS(value), INST_IMM(value), INST_RT(value));
    advance_PC();
}

void ORI(uint32_t value)
{
    OR_imm(INST_RS(value), INST_IMM(value), INST_RT(value));
    advance_PC();
}

void XORI(uint32_t value)
{
    XOR_imm(INST_RS(value), INST_IMM(value), INST_RT(value));
    advance_PC();
}

void LB(uint32_t value)
{
    write_GPR((char)read_uint8((uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value)), INST_RT(value));
    advance_PC();
}

void LBU(uint32_t value)
{
    write_GPR((uint8_t)read_uint8((uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value)), INST_RT(value));
    advance_PC();
}

void LD(uint32_t value)
{
    write_GPR(read_uint64((uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value)), INST_RT(value));
    advance_PC();
}

void LDL(uint32_t value)
{
    undefined_inst_error(value);
}

void LDR(uint32_t value)
{
    undefined_inst_error(value);
}

void LH(uint32_t value)
{
    write_GPR((short)read_uint16((uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value)), INST_RT(value));
    advance_PC();
}

void LHU(uint32_t value)
{
    write_GPR((uint16_t)read_uint16((uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value)), INST_RT(value));
    advance_PC();
}

void LL(uint32_t value)
{
    undefined_inst_error(value);
}

void LLD(uint32_t value)
{
    undefined_inst_error(value);
}

void LUI(uint32_t value)
{
    write_GPR((uint32_t)INST_IMM(value) << 16, INST_RT(value));
    advance_PC();
}

void LW(uint32_t value)
{
    write_GPR((uint32_t)read_uint32((uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value)), INST_RT(value));
    advance_PC();
}

void LWL(uint32_t value)
{
    short    offs = (short)INST_IMM(value);
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value)) + offs;
    uint32_t mem  = (uint32_t)read_uint32(addr);

    uint16_t shift = (offs % 8) * 8;
    uint64_t reg   = read_GPR(INST_RT(value));
    uint64_t res   = ((int)mem) & 0xFFFFFFFF00000000;
    res           |= ((mem << shift) | (reg & ((uint64_t)0xFFFFFFFF >> (32 - shift)))) & 0xFFFFFFFF;
    write_GPR(res, INST_RT(value));

    advance_PC();
}

void LWR(uint32_t value)
{
    short    offs = (short)INST_IMM(value);
    uint32_t addr = (uint32_t)read_GPR(INST_RS(value)) + offs;
    uint32_t mem  = (uint32_t)read_uint32(addr);

    uint16_t shift = (offs % 8) * 8;
    uint64_t reg   = read_GPR(INST_RT(value));
    uint64_t res   = 0xFFFFFFFF00000000;
    res           |= ((mem >> (24 - shift)) | (reg & ((uint64_t)0xFFFFFFFF << (shift + 8)))) & 0xFFFFFFFF;
    write_GPR(res, INST_RT(value));

    advance_PC();
}

void LWU(uint32_t value)
{
    write_GPR((uint32_t)read_uint32((uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value)), INST_RT(value));
    advance_PC();
}

void SB(uint32_t value)
{
    write_uint8(read_GPR(INST_RT(value)), (uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value));
    advance_PC();
}

void SC(uint32_t value)
{
    undefined_inst_error(value);
}

void SCD(uint32_t value)
{
    undefined_inst_error(value);
}

void SD(uint32_t value)
{
    write_uint64(read_GPR(INST_RT(value)), (uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value));
    advance_PC();
}

void SDL(uint32_t value)
{
    undefined_inst_error(value);
}

void SDR(uint32_t value)
{
    undefined_inst_error(value);
}

void SH(uint32_t value)
{
    write_uint16(read_GPR(INST_RT(value)), (uint32_t)read_GPR(INST_RS(value)) + (short)INST_IMM(value));
    advance_PC();
}

void SLTI(uint32_t value)
{
    SET_cond(INST_RT(value), (long)read_GPR(INST_RS(value)) < (short)INST_IMM(value));
    advance_PC();
}

void SLTIU(uint32_t value)
{
    SET_cond(INST_RT(value), read_GPR(INST_RS(value)) < (uint16_t)INST_IMM(value));
    advance_PC();
}

void SW(uint32_t value)
{
    write_uint32(read_GPR(INST_RT(value)), read_GPR(INST_RS(value)) + (short)INST_IMM(value));
    advance_PC();
}

void SWL(uint32_t value)
{
    undefined_inst_error(value);
}

void SWR(uint32_t value)
{
    undefined_inst_error(value);
}

void BEQ(uint32_t value)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value)) == (int)read_GPR(INST_RT(value)));
    advance_PC();
}

void BEQL(uint32_t value)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value)) == (int)read_GPR(INST_RT(value)));
    advance_PC();
}

void BGTZ(uint32_t value)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value)) > 0);
    advance_PC();
}

void BGTZL(uint32_t value)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value)) > 0);
    advance_PC();
}

void BLEZ(uint32_t value)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value)) <= 0);
    advance_PC();
}

void BLEZL(uint32_t value)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value)) <= 0);
    advance_PC();
}

void BNE(uint32_t value)
{
    BRANCH_cond(INST_IMM(value), (int)read_GPR(INST_RS(value)) != (int)read_GPR(INST_RT(value)));
    advance_PC();
}

void BNEL(uint32_t value)
{
    BRANCH_cond_likely(INST_IMM(value), (int)read_GPR(INST_RS(value)) != (int)read_GPR(INST_RT(value)));
    advance_PC();
}

void J(uint32_t value)
{
    JUMP_imm(INST_TARGET(value));
    advance_PC();
}

void JAL(uint32_t value)
{
    link_PC();
    JUMP_imm(INST_TARGET(value));
    advance_PC();
}

void CACHE(uint32_t value)
{
    advance_PC(); // Does nothing (we don't need to emulate this)
}