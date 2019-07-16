#include "r4300/cpu.h"

#include "common.h"

#include "SDL2/SDL.h"

void* CPU_run(void* vargp)
{
    while (is_running)
        interp_step();
    return NULL;
}

void COP0_WIRED_REG_WRITE_EVENT(void)
{
    regs.COP0[COP0_RANDOM].value = 0x1F;
}

void CPU_init(void* ROM, size_t ROM_size)
{
    memory_init(ROM, ROM_size);

    uint32_t rom_type   = (uint32_t)ROM_GAMEPACK;
    uint32_t reset_type = 0;
    uint32_t os_version = 0;
    uint32_t tv_type    = (uint32_t)config.region;

    regs.GPR[1].value  = 0x0000000000000001;
    regs.GPR[2].value  = 0x000000000EBDA536;
    regs.GPR[3].value  = 0x000000000EBDA536;
    regs.GPR[4].value  = 0x000000000000A536;
    regs.GPR[5].value  = 0xFFFFFFFFC0F1D859;
    regs.GPR[6].value  = 0xFFFFFFFFA4001F0C;
    regs.GPR[7].value  = 0xFFFFFFFFA4001F08;
    regs.GPR[8].value  = 0x00000000000000C0;
    regs.GPR[10].value = 0x0000000000000040;
    regs.GPR[11].value = 0xFFFFFFFFA4000040;
    regs.GPR[12].value = 0xFFFFFFFFED10D0B3;
    regs.GPR[13].value = 0x000000001402A4CC;
    regs.GPR[14].value = 0x000000002DE108EA;
    regs.GPR[15].value = 0x000000003103E121;
    regs.GPR[19].value = rom_type;
    regs.GPR[20].value = tv_type;
    regs.GPR[21].value = reset_type;
    regs.GPR[22].value = (get_CIC_seed() >> 8) & 0xFF;
    regs.GPR[23].value = os_version;
    regs.GPR[25].value = 0xFFFFFFFF9DEBB54F;
    regs.GPR[29].value = 0xFFFFFFFFA4001FF0;
    regs.GPR[31].value = 0xFFFFFFFFA4001550;
    regs.HI.value      = 0x000000003FC18657;
    regs.LO.value      = 0x000000003103E121;
    regs.PC.value      = 0xA4000040;

    regs.COP0[COP0_WIRED].write_callback = COP0_WIRED_REG_WRITE_EVENT;

    memory_memcpy(0xA4000040, 0x10000040, 0xFC0);

    regs.COP0[COP0_COMPARE].value = 0xFFFFFFFF;
    regs.COP0[COP0_STATUS].value  = 0x34000000;
    regs.COP0[COP0_CONFIG].value  = 0x0006E463;
    regs.COP0[COP0_RANDOM].value  = 0x1F;

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

    opcode_table_init();

    is_running = true;

    pthread_t CPU_thread;

    pthread_create(&CPU_thread, NULL, CPU_run, NULL);

    RDP_init();
}

void CPU_cleanup(void)
{
    memory_cleanup();
}