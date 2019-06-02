#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t PFN0;
    uint8_t  PageCoherency0;
    uint8_t  Dirty0;
    uint8_t  Valid0;
    uint8_t  Global0;
    uint32_t PFN1;
    uint8_t  PageCoherency1;
    uint8_t  Dirty1;
    uint8_t  Valid1;
    uint8_t  Global1;
    uint32_t EntryHi;
    uint32_t PageMask;

    bool Written;
} tlbentry_t;

tlbentry_t TLBEntries[32];

uint32_t TLBTranslateAddress(uint32_t Addr);

void WriteTLBEntryIndexed(void);
void WriteTLBEntryRandom (void);

void ReadTLBEntry(void);

void ProbeTLB(void);