#include "common.h"

uint32_t TLB_translate_address(uint32_t addr)
{
    if ((addr & 0xC0000000) == 0x80000000)
        return addr;

    for (size_t i = 0; i < 32; ++i)
    {
        tlbentry_t entry = TLB_entries[i];
        if (!entry.written) continue;

        uint32_t VPN = entry.entry_hi & ~(entry.page_mask | 0x1FFF);

        if ((addr & (VPN | 0xE0000000)) != VPN) continue;

        uint32_t mask      = (entry.page_mask >> 1) | 0x0FFF;
        uint32_t page_size = mask + 1;

        bool odd = (addr & page_size) == 1;

        uint32_t valid = (!odd) ? entry.valid0 : entry.valid1;

        if (valid == 0) continue;

        uint32_t PFN = (!odd) ? entry.PFN0 : entry.PFN1;

        uint32_t result = (PFN * page_size) | (addr & mask);

        return result;
    }

    return addr;
}

void write_TLB_entry(uint32_t index)
{
    tlbentry_t res;

    res.PFN0            = (uint32_t)((r4300.regs.COP0[COP0_ENTRYLO0].value & 0x3FFFFFC0) >> 6);
    res.valid0          = (uint8_t) ((r4300.regs.COP0[COP0_ENTRYLO0].value & 0b000010)   >> 1);
    res.dirty0          = (uint8_t) ((r4300.regs.COP0[COP0_ENTRYLO0].value & 0b000100)   >> 2);
    res.page_coherency0 = (uint8_t) ((r4300.regs.COP0[COP0_ENTRYLO0].value & 0b111000)   >> 3);
    res.PFN1            = (uint32_t)((r4300.regs.COP0[COP0_ENTRYLO1].value & 0x3FFFFFC0) >> 6);
    res.valid1          = (uint8_t) ((r4300.regs.COP0[COP0_ENTRYLO1].value & 0b000010)   >> 1);
    res.dirty1          = (uint8_t) ((r4300.regs.COP0[COP0_ENTRYLO1].value & 0b000100)   >> 2);
    res.page_coherency1 = (uint8_t) ((r4300.regs.COP0[COP0_ENTRYLO1].value & 0b111000)   >> 3);
    res.entry_hi        = (uint32_t)  r4300.regs.COP0[COP0_ENTRYHI].value;
    res.page_mask       = (uint32_t)  r4300.regs.COP0[COP0_PAGEMASK].value;
    res.global0         = (uint8_t)(((uint8_t)r4300.regs.COP0[COP0_ENTRYLO0].value & 0x1) & ((uint8_t)r4300.regs.COP0[COP0_ENTRYLO1].value & 0x1));
    res.global1         = res.global0;

    TLB_entries[index & 0x1F] = res;
}

void write_TLB_entry_indexed(void)
{
    write_TLB_entry((uint32_t)r4300.regs.COP0[COP0_INDEX].value);
}

void write_TLB_entry_random(void)
{
    write_TLB_entry((uint32_t)r4300.regs.COP0[COP0_RANDOM].value);
}

void read_TLB_entry(void)
{
    tlbentry_t entry = TLB_entries[(uint32_t)r4300.regs.COP0[COP0_INDEX].value & 0x1F];
    r4300.regs.COP0[COP0_ENTRYLO0].value = (uint32_t)((entry.PFN0 << 6)
                                                | (uint8_t)(entry.global0 & 0x1)
                                                | (uint8_t)((entry.valid0 & 0x1) << 1)
                                                | (uint8_t)((entry.dirty0 & 0x1) << 2)
                                                | (uint8_t)((entry.page_coherency0 & 0b111) << 3));
    r4300.regs.COP0[COP0_ENTRYLO1].value = (uint32_t)((entry.PFN1 << 6)
                                                | (uint8_t)(entry.global1 & 0x1)
                                                | (uint8_t)((entry.valid1 & 0x1) << 1)
                                                | (uint8_t)((entry.dirty1 & 0x1) << 2)
                                                | (uint8_t)((entry.page_coherency1 & 0b111) << 3));
    r4300.regs.COP0[COP0_PAGEMASK].value = (uint32_t)(entry.page_mask << 13);
    r4300.regs.COP0[COP0_ENTRYHI].value  = entry.entry_hi;
}

void probe_TLB(void)
{
    bool found_entry = false;
    for (size_t i = 0; i < 32; ++i)
    {
        tlbentry_t entry = TLB_entries[i];

        if ((entry.valid0 | entry.valid1) == 0) continue;

        uint32_t entry_hi   = (uint32_t)r4300.regs.COP0[COP0_ENTRYHI].value;
        uint32_t VPN2       = (entry_hi & 0xFFFFE000) >> 13;
        uint32_t ASID       =  entry_hi & 0xFF;
        uint32_t entry_VPN2 = (entry.entry_hi & 0xFFFFE000) >> 13;
        uint32_t entry_ASID =  entry.entry_hi & 0xFF;

        if (entry_VPN2 == VPN2 && entry_ASID == ASID)
        {
            found_entry = true;
            r4300.regs.COP0[COP0_INDEX].value = i & 0x1F;
            break;
        }
    }

    if (!found_entry) r4300.regs.COP0[COP0_INDEX].value |= 0x80000000;
}