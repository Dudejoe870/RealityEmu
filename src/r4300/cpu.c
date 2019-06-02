#include "cpu.h"

#include <pthread.h>
#include <stdio.h>
#include <byteswap.h>

#include "mem.h"
#include "cic.h"
#include "opcodetable.h"
#include "interpreter.h"
#include "config.h"

#define MEASURE_MHZ

#ifdef MEASURE_MHZ
#include <time.h>

double CPUMHz = 0;
#endif

#ifdef MEASURE_MHZ
void* MeasureMHz(void* vargp)
{
    uint32_t Count = 0;
    while (IsRunning)
    {
        if (Count >= 10000)
        {
            float TimeSeconds = ((float)clock()) / CLOCKS_PER_SEC;

            CPUMHz = ((long)GetAllCycles() / 1000000) / TimeSeconds;
            Count = 0;
        }
        ++Count;
    }
    return NULL;
}
#endif

void* RunCPU(void* vargp)
{
    #ifdef MEASURE_MHZ
    pthread_t MeasureThread;

    pthread_create(&MeasureThread, NULL, MeasureMHz, NULL);
    #endif
    while (IsRunning)
    {
        Step();
    }
    #ifdef MEASURE_MHZ
    printf("Finished: CPU is running at %fMHz\n", CPUMHz);
    #endif
    return NULL;
}

void COP0_WIRED_REG_WRITE_EVENT(void)
{
    Regs.COP0[COP0_Random].Value = 0x1F;
}

void CPUInit(void* ROM, size_t ROMSize)
{
    MemoryInit(ROM, ROMSize);

    uint32_t RomType   = 0;
    uint32_t ResetType = 0;
    uint32_t osVersion = 0;
    uint32_t TVType = (uint32_t)Config.Region;

    Regs.GPR[1].Value  = 0x0000000000000001;
    Regs.GPR[2].Value  = 0x000000000EBDA536;
    Regs.GPR[3].Value  = 0x000000000EBDA536;
    Regs.GPR[4].Value  = 0x000000000000A536;
    Regs.GPR[5].Value  = 0xFFFFFFFFC0F1D859;
    Regs.GPR[6].Value  = 0xFFFFFFFFA4001F0C;
    Regs.GPR[7].Value  = 0xFFFFFFFFA4001F08;
    Regs.GPR[8].Value  = 0x00000000000000C0;
    Regs.GPR[10].Value = 0x0000000000000040;
    Regs.GPR[11].Value = 0xFFFFFFFFA4000040;
    Regs.GPR[12].Value = 0xFFFFFFFFED10D0B3;
    Regs.GPR[13].Value = 0x000000001402A4CC;
    Regs.GPR[14].Value = 0x000000002DE108EA;
    Regs.GPR[15].Value = 0x000000003103E121;
    Regs.GPR[19].Value = RomType;
    Regs.GPR[20].Value = TVType;
    Regs.GPR[21].Value = ResetType;
    Regs.GPR[22].Value = (GetCICSeed() >> 8) & 0xFF;
    Regs.GPR[23].Value = osVersion;
    Regs.GPR[25].Value = 0xFFFFFFFF9DEBB54F;
    Regs.GPR[29].Value = 0xFFFFFFFFA4001FF0;
    Regs.GPR[31].Value = 0xFFFFFFFFA4001550;
    Regs.HI.Value      = 0x000000003FC18657;
    Regs.LO.Value      = 0x000000003103E121;
    Regs.PC.Value      = 0xA4000040;

    Regs.COP0[COP0_Wired].WriteCallback = COP0_WIRED_REG_WRITE_EVENT;

    MemoryCopy(0xA4000040, 0x10000040, 0xFC0);

    Regs.COP0[COP0_Compare].Value = 0xFFFFFFFF;
    Regs.COP0[COP0_Status].Value  = 0x34000000;
    Regs.COP0[COP0_Config].Value  = 0x0006E463;
    Regs.COP0[COP0_Random].Value  = 0x1F;

    RI_SELECT_REG_RW = bswap_32(0b1110);
    VI_INTR_REG_RW   = bswap_32(1023);
    VI_H_SYNC_REG_RW = bswap_32(0xD1);
    VI_V_SYNC_REG_RW = bswap_32(0xD2047);

    uint32_t BSD_DOM1_CONFIG = ReadUInt32(0x10000000);

    PI_BSD_DOM1_LAT_REG_RW = bswap_32((BSD_DOM1_CONFIG      ) & 0xFF);
    PI_BSD_DOM1_PWD_REG_RW = bswap_32((BSD_DOM1_CONFIG >> 8 ) & 0xFF);
    PI_BSD_DOM1_PGS_REG_RW = bswap_32((BSD_DOM1_CONFIG >> 16) & 0xFF);
    PI_BSD_DOM1_RLS_REG_RW = bswap_32((BSD_DOM1_CONFIG >> 20) & 0x03);

    OpcodeTableInit();

    IsRunning = true;

    pthread_t CPUThread;

    pthread_create(&CPUThread, NULL, RunCPU, NULL);
}

void CPUDeInit(void)
{
    MemoryDeInit();
}