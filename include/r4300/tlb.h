#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t PFN0;
    uint8_t  page_coherency0;
    uint8_t  dirty0;
    uint8_t  valid0;
    uint8_t  global0;
    uint32_t PFN1;
    uint8_t  page_coherency1;
    uint8_t  dirty1;
    uint8_t  valid1;
    uint8_t  global1;
    uint32_t entry_hi;
    uint32_t page_mask;

    bool written;
} tlbentry_t;

tlbentry_t TLB_entries[32];

uint32_t TLB_translate_address(uint32_t addr);

void write_TLB_entry_indexed(void);
void write_TLB_entry_random (void);

void read_TLB_entry(void);

void probe_TLB(void);