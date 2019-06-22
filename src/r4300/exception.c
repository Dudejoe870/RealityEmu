#include "exception.h"

#include "cpu.h"
#include "interpreter.h"
#include "../config.h"

#include <stdio.h>

typedef enum
{
    EXC_INT = 0,
    EXC_MOD = 1 << 2,
    EXC_TLBL = 2 << 2,
    EXC_TLBS = 3 << 2,
    EXC_ADEL = 4 << 2,
    EXC_ADES = 5 << 2,
    EXC_IBE = 6 << 2,
    EXC_DBE = 7 << 2,
    EXC_SYS = 8 << 2,
    EXC_BP = 9 << 2,
    EXC_RI = 10 << 2,
    EXC_CPU = 11 << 2,
    EXC_OV = 12 << 2,
    EXC_TR = 13 << 2,
    EXC_FPE = 15 << 2,
    EXC_WATCH = 23 << 2
} excode_t;

void invoke_TLB_miss(uint32_t addr, bool store)
{
    uint32_t VPN2 = (addr / 0x1000) >> 1;
    write_COP0(addr,       COP0_BADVADDR);
    write_COP0(VPN2 << 4,  COP0_CONTEXT);
    write_COP0(VPN2 << 4,  COP0_XCONTEXT);
    write_COP0(VPN2 << 13, COP0_ENTRYHI);

    write_COP0(!get_is_branching() ? read_PC() : read_PC() - 4, COP0_EPC);
    write_COP0(read_COP0(COP0_CAUSE)
              | ((store ? (uint32_t)EXC_TLBS : (uint32_t)EXC_TLBL) 
              | (get_is_branching() ? 0x80000000 : 0)), COP0_CAUSE);
    write_COP0(read_COP0(COP0_CAUSE) & ~0xFF00, COP0_CAUSE);
    if (config.debug_logging) printf("TLB Miss at PC: 0x%x, BadVAddr: 0x%x\n", read_PC(), addr);

    write_PC(((read_COP0(COP0_STATUS) & 0b10000000000000000000000) > 0) ? 0xBFC00200 - 4 : 0x80000000 - 4);
    // We use the Exception Vector - 4 here because if this is a Load or store instruction
    // we are going to advance by 4, so to fix this we just set it to its destination - 4.
}

void invoke_break(void)
{
    write_COP0(!get_is_branching() ? read_PC() : read_PC() - 4, COP0_EPC);
    write_COP0(read_COP0(COP0_CAUSE)
              | (uint32_t)EXC_BP
              | (get_is_branching() ? 0x80000000 : 0), COP0_CAUSE);
    write_COP0(read_COP0(COP0_CAUSE) & ~0xFF00, COP0_CAUSE);
    if (config.debug_logging) printf("Break at PC: 0x%x\n", read_PC());

    write_PC(((read_COP0(COP0_STATUS) & 0b10000000000000000000000) > 0) ? 0xBFC00380 - 4 : 0x80000180 - 4);
}

void invoke_trap(void)
{
    write_COP0(!get_is_branching() ? read_PC() : read_PC() - 4, COP0_EPC);
    write_COP0(read_COP0(COP0_CAUSE)
              | (uint32_t)EXC_TR
              | (get_is_branching() ? 0x80000000 : 0), COP0_CAUSE);
    write_COP0(read_COP0(COP0_CAUSE) & ~0xFF00, COP0_CAUSE);
    if (config.debug_logging) printf("Trap at PC: 0x%x\n", read_PC());

    write_PC(((read_COP0(COP0_STATUS) & 0b10000000000000000000000) > 0) ? 0xBFC00380 - 4 : 0x80000180 - 4);
}

void poll_int(void)
{
    if ((uint32_t)regs.COP0[COP0_COUNT].value == (uint32_t)regs.COP0[COP0_COMPARE].value)
        regs.COP0[COP0_CAUSE].value |= 0x8000;
    
    if (((uint32_t)regs.COP0[COP0_STATUS].value & 0b111) == 0b001)
    {
        uint32_t Cause = (uint32_t)regs.COP0[COP0_CAUSE].value & 0xFF00;
        if (Cause > 0)
        {
            uint32_t Status = (uint32_t)regs.COP0[COP0_STATUS].value & 0xFF00;
            if ((Status & Cause) > 0)
            {
                regs.COP0[COP0_EPC].value    = !get_is_branching() ? (uint32_t)regs.PC.value : (uint32_t)regs.PC.value - 4;
                regs.COP0[COP0_CAUSE].value |= (uint32_t)EXC_INT | (get_is_branching() ? 0x80000000 : 0);
                regs.COP0[COP0_CAUSE].value &= ~0xFF00;

                if (config.debug_logging) printf("Interrupt at PC: 0x%x\n", (uint32_t)regs.PC.value);

                regs.PC.value = (uint32_t)((regs.COP0[COP0_STATUS].value & 0b10000000000000000000000) > 0) ? 0xBFC00380 : 0x80000180;
            }
        }
    }
}