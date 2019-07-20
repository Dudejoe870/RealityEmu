#include "r4300/mi.h"

#include "common.h"

void invoke_mi_interrupt(uint8_t interrupt)
{
    if ((bswap_32(MI_INTR_MASK_REG_R) & (uint32_t)(1 << interrupt)) > 0)
    {
        uint32_t value = (uint32_t)(1 << interrupt) | bswap_32(MI_INTR_REG_R);
        MI_INTR_REG_R = bswap_32(value);
        r4300.regs.COP0[COP0_CAUSE].value |= 0x400;
    }
}