#include "r4300/mi.h"

#include "common.h"

void invoke_mi_interrupt(uint8_t interrupt)
{
    uint32_t value = (uint32_t)(1 << interrupt);
    MI_INTR_REG_R |= bswap_32(value);
    if ((bswap_32(MI_INTR_MASK_REG_R) & value) > 0)
    {
        r4300.regs.COP0[COP0_CAUSE].value |= 0x400;
    }

    if (vi_event != NULL) vi_event();
}