#include "tlb.h"

#include "cpu.h"

#include <stddef.h>

uint32_t TLBTranslateAddress(uint32_t Addr)
{
    if ((Addr & 0xC0000000) == 0x80000000)
        return Addr;
    
    for (size_t i = 0; i < 32; ++i)
    {
        tlbentry_t Entry = TLBEntries[i];
        if (!Entry.Written) continue;

        uint32_t VPN = Entry.EntryHi & ~(Entry.PageMask | 0x1FFF);

        if ((Addr & (VPN | 0xE0000000)) != VPN) continue;

        uint32_t Mask     = (Entry.PageMask >> 1) | 0x0FFF;
        uint32_t PageSize = Mask + 1;

        bool Odd = (Addr & PageSize) == 1;

        uint32_t Valid = (!Odd) ? Entry.Valid0 : Entry.Valid1;

        if (Valid == 0) continue;

        uint32_t PFN = (!Odd) ? Entry.PFN0 : Entry.PFN1;

        uint32_t Result = (PFN * PageSize) | (Addr & Mask);

        return Result;
    }

    return Addr;
}

void WriteTLBEntry(uint32_t Index)
{
    tlbentry_t Res;

    Res.PFN0           = (uint32_t)((Regs.COP0[COP0_EntryLo0].Value & 0x3FFFFFC0) >> 6);
    Res.Valid0         = (uint8_t) ((Regs.COP0[COP0_EntryLo0].Value & 0b000010)   >> 1);
    Res.Dirty0         = (uint8_t) ((Regs.COP0[COP0_EntryLo0].Value & 0b000100)   >> 2);
    Res.PageCoherency0 = (uint8_t) ((Regs.COP0[COP0_EntryLo0].Value & 0b111000)   >> 3);
    Res.PFN1           = (uint32_t)((Regs.COP0[COP0_EntryLo1].Value & 0x3FFFFFC0) >> 6);
    Res.Valid1         = (uint8_t) ((Regs.COP0[COP0_EntryLo1].Value & 0b000010)   >> 1);
    Res.Dirty1         = (uint8_t) ((Regs.COP0[COP0_EntryLo1].Value & 0b000100)   >> 2);
    Res.PageCoherency1 = (uint8_t) ((Regs.COP0[COP0_EntryLo1].Value & 0b111000)   >> 3);
    Res.EntryHi        = (uint32_t)  Regs.COP0[COP0_EntryHi].Value;
    Res.PageMask       = (uint32_t)  Regs.COP0[COP0_PageMask].Value;
    Res.Global0        = (uint8_t) (((uint8_t)Regs.COP0[COP0_EntryLo0].Value & 0x1) & ((uint8_t)Regs.COP0[COP0_EntryLo1].Value & 0x1));
    Res.Global1        = Res.Global0;

    TLBEntries[Index & 0x1F] = Res;
}

void WriteTLBEntryIndexed(void)
{
    WriteTLBEntry((uint32_t)Regs.COP0[COP0_Index].Value);
}

void WriteTLBEntryRandom(void)
{
    WriteTLBEntry((uint32_t)Regs.COP0[COP0_Random].Value);
}

void ReadTLBEntry(void)
{
    tlbentry_t Entry = TLBEntries[(uint32_t)Regs.COP0[COP0_Index].Value & 0x1F];
    Regs.COP0[COP0_EntryLo0].Value = (uint32_t)((Entry.PFN0 << 6)
                                                | (uint8_t)(Entry.Global0 & 0x1)
                                                | (uint8_t)((Entry.Valid0 & 0x1) << 1)
                                                | (uint8_t)((Entry.Dirty0 & 0x1) << 2)
                                                | (uint8_t)((Entry.PageCoherency0 & 0b111) << 3));
    Regs.COP0[COP0_EntryLo1].Value = (uint32_t)((Entry.PFN1 << 6)
                                                | (uint8_t)(Entry.Global1 & 0x1)
                                                | (uint8_t)((Entry.Valid1 & 0x1) << 1)
                                                | (uint8_t)((Entry.Dirty1 & 0x1) << 2)
                                                | (uint8_t)((Entry.PageCoherency1 & 0b111) << 3));
    Regs.COP0[COP0_PageMask].Value = (uint32_t)(Entry.PageMask << 13);
    Regs.COP0[COP0_EntryHi].Value  = Entry.EntryHi;
}

void ProbeTLB(void)
{
    bool FoundEntry = false;
    for (size_t i = 0; i < 32; ++i)
    {
        tlbentry_t Entry = TLBEntries[i];

        if ((Entry.Valid0 | Entry.Valid1) == 0) continue;

        uint32_t EntryHi = (uint32_t)Regs.COP0[COP0_EntryHi].Value;
        uint32_t VPN2    = (EntryHi & 0xFFFFE000) >> 13;
        uint32_t ASID    = EntryHi & 0xFF;
        uint32_t EntryVPN2    = (Entry.EntryHi & 0xFFFFE000) >> 13;
        uint32_t EntryASID    = Entry.EntryHi & 0xFF;

        if (EntryVPN2 == VPN2 && EntryASID == ASID)
        {
            FoundEntry = true;
            Regs.COP0[COP0_Index].Value = i & 0x1F;
            break;
        }
    }

    if (!FoundEntry) Regs.COP0[COP0_Index].Value |= 0x80000000;
}