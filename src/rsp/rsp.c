#include "rsp/rsp.h"

#include "common.h"

__attribute__((__always_inline__)) static inline void RSP_interp_step(void)
{
    while (bswap_32(SP_STATUS_REG_R) & 1);
    rsp.regs.PC.value = bswap_32(SP_PC_REG_RW);

    bool should_branch = rsp.is_branching;

    if (rsp.regs.PC.value > 0xFFF)
    {
        rsp.regs.PC.value = 0;
        SP_STATUS_REG_R |= bswap_32(1);
        return;
    }

    uint32_t inst = bswap_32(((uint32_t*)SP_IMEM_RW)[rsp.regs.PC.value & 0xFFF]);
    opcode_t op   = RSP_opcode_table[INST_OP(inst)];

    if (op.interpret == NULL)
    {
        MIPS_undefined_inst_error(inst, &rsp);
        return;
    }

    rsp.regs.GPR[0].value = 0;

    rsp.curr_inst_cycles = 1;
    op.interpret(inst, &rsp);

    rsp.cycles     += rsp.curr_inst_cycles;
    rsp.all_cycles += rsp.curr_inst_cycles;

    if (!is_running) return;

    if (should_branch) // If we should branch (aka the last instruction was a branch instruction).
    {
        rsp.regs.PC.value = rsp.curr_target; // Then set to the PC the target.
        rsp.is_branching  = false; // And reset the variables.
        rsp.curr_target   = 0;
    }

    SP_PC_REG_RW = bswap_32(rsp.regs.PC.value);
}

void* RSP_run(void* vargp)
{
    while (is_running)
        RSP_interp_step();
    return NULL;
}

void RSP_init(void)
{
    rsp.rsp = true;

    RSP_opcode_table_init();
}

void RSP_cleanup(void)
{
    
}

void RSP_start(void)
{
    pthread_t RSP_thread;

    pthread_create(&RSP_thread, NULL, RSP_run, NULL);

    if (config.debug_logging) puts("RSP: RSP started.");

    RSP_has_started = true;
}