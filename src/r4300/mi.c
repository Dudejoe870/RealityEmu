#include "mi.h"

#include "mem.h"
#include "cpu.h"

#include <stdio.h>
#include <byteswap.h>

void InvokeMIInterrupt(uint8_t Interrupt)
{
    if ((bswap_32(MI_INTR_MASK_REG_R) & (uint32_t)(1 << Interrupt)) > 0)
    {
        uint32_t Value = (uint32_t)(1 << Interrupt) | bswap_32(MI_INTR_REG_R);
        MI_INTR_REG_R = bswap_32(Value);
        Regs.COP0[COP0_Cause].Value |= 0x400;
    }
}