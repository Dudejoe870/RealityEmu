#include "rsp/rsp.h"

#include "common.h"

__attribute__((__always_inline__)) static inline void RSP_interp_step(void)
{
    if ((bswap_32(SP_STATUS_REG_R) & 1) || !is_running) return;
    rsp.regs.PC.value = bswap_32(SP_PC_REG_RW);

    bool should_branch = rsp.is_branching;

    if ((rsp.regs.PC.value + 4) > 0xFFF)
    {
        rsp.regs.PC.value = 0;
        SP_STATUS_REG_R |= bswap_32(1);
        return;
    }

    uint32_t inst = bswap_32(*(uint32_t*)&(((uint8_t*)SP_IMEM_RW)[rsp.regs.PC.value & 0xFFF]));
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

void RSP_run(void)
{
    if ((bswap_32(SP_STATUS_REG_R) & 1) || !is_running) return;
    while (!(bswap_32(SP_STATUS_REG_R) & 1) && is_running)
    {
        RSP_interp_step();
    }
}

void COP0_DMA_CACHE_REG_WRITE_EVENT(uint64_t value, uint8_t index)
{
    write_uint32((uint32_t)value, 0xA4040000); // SP_MEM_ADDR_REG
}

void COP0_DMA_CACHE_REG_READ_EVENT(uint8_t index)
{
    rsp.regs.COP0[index].value = read_uint32(0xA4040000); // SP_MEM_ADDR_REG
}

void COP0_DMA_DRAM_REG_WRITE_EVENT(uint64_t value, uint8_t index)
{
    write_uint32((uint32_t)value, 0xA4040004); // SP_DRAM_ADDR_REG
}

void COP0_DMA_DRAM_REG_READ_EVENT(uint8_t index)
{
    rsp.regs.COP0[index].value = read_uint32(0xA4040004); // SP_DRAM_ADDR_REG
}

void COP0_DMA_READ_LENGTH_REG_WRITE_EVENT(uint64_t value, uint8_t index)
{
    write_uint32((uint32_t)value, 0xA4040008); // SP_RD_LEN_REG
}

void COP0_DMA_READ_LENGTH_REG_READ_EVENT(uint8_t index)
{
    rsp.regs.COP0[index].value = read_uint32(0xA4040008); // SP_RD_LEN_REG
}

void COP0_DMA_WRITE_LENGTH_REG_WRITE_EVENT(uint64_t value, uint8_t index)
{
    write_uint32((uint32_t)value, 0xA404000C); // SP_WR_LEN_REG
}

void COP0_DMA_WRITE_LENGTH_REG_READ_EVENT(uint8_t index)
{
    rsp.regs.COP0[index].value = read_uint32(0xA404000C); // SP_WR_LEN_REG
}

void RSP_init(void)
{
    rsp.rsp = true;

    rsp.regs.COP0[COP0_DMA_CACHE].write_callback = COP0_DMA_CACHE_REG_WRITE_EVENT;
    rsp.regs.COP0[COP0_DMA_CACHE].read_callback  = COP0_DMA_CACHE_REG_READ_EVENT;

    rsp.regs.COP0[COP0_DMA_DRAM].write_callback = COP0_DMA_DRAM_REG_WRITE_EVENT;
    rsp.regs.COP0[COP0_DMA_DRAM].read_callback  = COP0_DMA_DRAM_REG_READ_EVENT;

    rsp.regs.COP0[COP0_DMA_READ_LENGTH].write_callback = COP0_DMA_READ_LENGTH_REG_WRITE_EVENT;
    rsp.regs.COP0[COP0_DMA_READ_LENGTH].read_callback  = COP0_DMA_READ_LENGTH_REG_READ_EVENT;

    rsp.regs.COP0[COP0_DMA_WRITE_LENGTH].write_callback = COP0_DMA_WRITE_LENGTH_REG_WRITE_EVENT;
    rsp.regs.COP0[COP0_DMA_WRITE_LENGTH].read_callback  = COP0_DMA_WRITE_LENGTH_REG_READ_EVENT;

    RSP_opcode_table_init();
}

void RSP_cleanup(void)
{
    
}