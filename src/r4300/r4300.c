#include "r4300/r4300.h"

#include "common.h"

uint32_t current_scanline = 0;
uint32_t vi_cycle_count   = 0;

__attribute__((__always_inline__)) static inline void CPU_interp_step(void)
{
    bool should_branch = r4300.is_branching;

    uint32_t inst = bswap_32(*(uint32_t*)get_real_memory_loc((uint32_t)r4300.regs.PC.value));
    opcode_t op   = CPU_opcode_table[INST_OP(inst)];

    if (op.interpret == NULL)
    {
        MIPS_undefined_inst_error(inst, &r4300);
        return;
    }

    r4300.regs.GPR[0].value = 0;

    if ((uint32_t)r4300.regs.COP0[COP0_COUNT].value >= 0xFFFFFFFF)
    {
        r4300.regs.COP0[COP0_COUNT].value = 0;

        r4300.cycles = 0;
    }

    if (vi_cycle_count >= 6510)
    {
        if (current_scanline >= (bswap_32(VI_V_SYNC_REG_RW) & 0x3FF)) current_scanline = 0;
        else ++current_scanline;
        VI_CURRENT_REG_R = bswap_32(current_scanline);

        if (current_scanline == (bswap_32(VI_INTR_REG_RW) - 1))
        {
            invoke_mi_interrupt(MI_INTR_VI);
            ++VI_intrs;
        }

        vi_cycle_count = 0;
    }

    r4300.curr_inst_cycles = 1;
    op.interpret(inst, &r4300);

    vi_cycle_count   += r4300.curr_inst_cycles;
    r4300.cycles     += r4300.curr_inst_cycles;
    r4300.all_cycles += r4300.curr_inst_cycles;

    r4300.regs.COP0[COP0_COUNT].value = (uint32_t)(r4300.cycles >> 1);
    if (r4300.regs.COP0[COP0_RANDOM].value < r4300.regs.COP0[COP0_WIRED].value 
     || r4300.regs.COP0[COP0_RANDOM].value == 0) r4300.regs.COP0[COP0_RANDOM].value = 0x1F;
    --r4300.regs.COP0[COP0_RANDOM].value;

    if (!is_running) return;

    if (should_branch) // If we should branch (aka the last instruction was a branch instruction).
    {
        r4300.regs.PC.value = r4300.curr_target; // Then set to the PC the target.
        r4300.is_branching  = false; // And reset the variables.
        r4300.curr_target   = 0;
    }
    
    poll_int();
}

void* CPU_run(void* vargp)
{
    while (is_running)
    {
        CPU_interp_step();
        RSP_run();
    }
    return NULL;
}

void COP0_WIRED_REG_WRITE_EVENT(uint64_t value, uint8_t index)
{
    r4300.regs.COP0[COP0_RANDOM].value = 0x1F;
}

void CPU_init(void* ROM, size_t ROM_size)
{
    memory_init(ROM, ROM_size);

    r4300.rsp = false;

    uint32_t rom_type   = (uint32_t)ROM_GAMEPACK;
    uint32_t reset_type = 0;
    uint32_t os_version = 0;
    uint32_t tv_type    = (uint32_t)config.region;

    r4300.regs.GPR[1].value  = 0x0000000000000001;
    r4300.regs.GPR[2].value  = 0x000000000EBDA536;
    r4300.regs.GPR[3].value  = 0x000000000EBDA536;
    r4300.regs.GPR[4].value  = 0x000000000000A536;
    r4300.regs.GPR[5].value  = 0xFFFFFFFFC0F1D859;
    r4300.regs.GPR[6].value  = 0xFFFFFFFFA4001F0C;
    r4300.regs.GPR[7].value  = 0xFFFFFFFFA4001F08;
    r4300.regs.GPR[8].value  = 0x00000000000000C0;
    r4300.regs.GPR[10].value = 0x0000000000000040;
    r4300.regs.GPR[11].value = 0xFFFFFFFFA4000040;
    r4300.regs.GPR[12].value = 0xFFFFFFFFED10D0B3;
    r4300.regs.GPR[13].value = 0x000000001402A4CC;
    r4300.regs.GPR[14].value = 0x000000002DE108EA;
    r4300.regs.GPR[15].value = 0x000000003103E121;
    r4300.regs.GPR[19].value = rom_type;
    r4300.regs.GPR[20].value = tv_type;
    r4300.regs.GPR[21].value = reset_type;
    r4300.regs.GPR[22].value = (get_CIC_seed() >> 8) & 0xFF;
    r4300.regs.GPR[23].value = os_version;
    r4300.regs.GPR[25].value = 0xFFFFFFFF9DEBB54F;
    r4300.regs.GPR[29].value = 0xFFFFFFFFA4001FF0;
    r4300.regs.GPR[31].value = 0xFFFFFFFFA4001550;
    r4300.regs.HI.value      = 0x000000003FC18657;
    r4300.regs.LO.value      = 0x000000003103E121;
    r4300.regs.PC.value      = 0xA4000040;

    r4300.regs.COP0[COP0_WIRED].write_callback = COP0_WIRED_REG_WRITE_EVENT;

    memory_memcpy(0xA4000040, 0x10000040, 0xFC0);

    r4300.regs.COP0[COP0_COMPARE].value = 0xFFFFFFFF;
    r4300.regs.COP0[COP0_STATUS].value  = 0x34000000;
    r4300.regs.COP0[COP0_CONFIG].value  = 0x0006E463;
    r4300.regs.COP0[COP0_RANDOM].value  = 0x1F;

    RI_SELECT_REG_RW = bswap_32(0b1110);
    VI_INTR_REG_RW   = bswap_32(1023);
    VI_H_SYNC_REG_RW = bswap_32(0xD1);
    VI_V_SYNC_REG_RW = bswap_32(0xD2047);

    uint32_t BSD_DOM1_CONFIG = read_uint32(0xB0000000);

    PI_BSD_DOM1_LAT_REG_RW = bswap_32((BSD_DOM1_CONFIG      ) & 0xFF);
    PI_BSD_DOM1_PWD_REG_RW = bswap_32((BSD_DOM1_CONFIG >> 8 ) & 0xFF);
    PI_BSD_DOM1_PGS_REG_RW = bswap_32((BSD_DOM1_CONFIG >> 16) & 0xFF);
    PI_BSD_DOM1_RLS_REG_RW = bswap_32((BSD_DOM1_CONFIG >> 20) & 0x03);

    RDRAM_CONFIG_REG       = bswap_32(0xB4190010);
    RDRAM_DELAY_REG        = bswap_32(0x2B3B1A0B);
    RDRAM_RAS_INTERVAL_REG = bswap_32(0x101C0A04);

    CPU_opcode_table_init();

    is_running = true;

    pthread_t CPU_thread;

    pthread_create(&CPU_thread, NULL, CPU_run, NULL);

    RDP_init();
    RSP_init();
}

void CPU_cleanup(void)
{
    memory_cleanup();
}