#include "exception.h"

#include "cpu.h"
#include "interpreter.h"

#include <stdio.h>

typedef enum
{
    Exc_Int = 0,
    Exc_Mod = 1 << 2,
    Exc_TLBL = 2 << 2,
    Exc_TLBS = 3 << 2,
    Exc_AdEL = 4 << 2,
    Exc_AdES = 5 << 2,
    Exc_IBE = 6 << 2,
    Exc_DBE = 7 << 2,
    Exc_Sys = 8 << 2,
    Exc_Bp = 9 << 2,
    Exc_RI = 10 << 2,
    Exc_CpU = 11 << 2,
    Exc_Ov = 12 << 2,
    Exc_Tr = 13 << 2,
    Exc_FPE = 15 << 2,
    Exc_WATCH = 23 << 2
} excode_t;

void InvokeTLBMiss(uint32_t Addr, bool Store)
{
    uint32_t VPN2 = (Addr / 0x1000) >> 1;
    WriteCOP0(Addr,       COP0_BadVAddr);
    WriteCOP0(VPN2 << 4,  COP0_Context);
    WriteCOP0(VPN2 << 4,  COP0_XContext);
    WriteCOP0(VPN2 << 13, COP0_EntryHi);

    WriteCOP0(GetIsBranching() ? ReadPC() : ReadPC() - 4, COP0_EPC);
    WriteCOP0(ReadCOP0(COP0_Cause)
              | ((Store ? (uint32_t)Exc_TLBS : (uint32_t)Exc_TLBL) 
              | (GetIsBranching() ? 0x80000000 : 0)), COP0_Cause);
    printf("TLB Miss at PC: 0x%x, BadVAddr: 0x%x\n", ReadPC(), Addr);

    WritePC(((ReadCOP0(COP0_Status) & 0b10000000000000000000000) > 0) ? 0xBFC00200 - 4 : 0x80000000 - 4);
    // We use the Exception Vector - 4 here because if this is a Load or Store instruction
    // we are going to advance by 4, so to fix this we just set it to its destination - 4.
}

void InvokeBreak(void)
{
    WriteCOP0(GetIsBranching() ? ReadPC() : ReadPC() - 4, COP0_EPC);
    WriteCOP0(ReadCOP0(COP0_Cause)
              | (uint32_t)Exc_Bp
              | (GetIsBranching() ? 0x80000000 : 0), COP0_Cause);
    printf("Break at PC: 0x%x\n", ReadPC());

    WritePC(((ReadCOP0(COP0_Status) & 0b10000000000000000000000) > 0) ? 0xBFC00380 - 4 : 0x80000180 - 4);
}

void InvokeTrap(void)
{
    WriteCOP0(GetIsBranching() ? ReadPC() : ReadPC() - 4, COP0_EPC);
    WriteCOP0(ReadCOP0(COP0_Cause)
              | (uint32_t)Exc_Tr
              | (GetIsBranching() ? 0x80000000 : 0), COP0_Cause);
    printf("Trap at PC: 0x%x\n", ReadPC());

    WritePC(((ReadCOP0(COP0_Status) & 0b10000000000000000000000) > 0) ? 0xBFC00380 - 4 : 0x80000180 - 4);
}