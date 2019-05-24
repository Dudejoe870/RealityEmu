#include "mem.h"

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <byteswap.h>

void* RDRAM;
void* SP_DMEM;

void MemoryInit(void* ROM, size_t ROMSize)
{
    size_t i = 0;

    size_t RDRAM_Size = (Config.ExpansionPak) ? 8388608 : 4194304;

    RDRAM = malloc(RDRAM_Size);
    MemEntries[i].Base          = 0x00000000;
    MemEntries[i].EndAddr       = 0x00000000 + (RDRAM_Size - 1);
    MemEntries[i].MemBlock      = RDRAM;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    SP_DMEM = malloc(0x1000);
    MemEntries[i].Base          = 0x04000000;
    MemEntries[i].EndAddr       = 0x04000FFF;
    MemEntries[i].MemBlock      = SP_DMEM;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x10000000;
    MemEntries[i].EndAddr       = 0x10000000 + (ROMSize - 1);
    MemEntries[i].MemBlock      = ROM;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;
}

void MemoryDeInit(void)
{
    for (size_t i = 0; i < MEMORY_ENTRIES; ++i)
    {
        mementry_t MemEntry = MemEntries[i];
        if (!MemEntry.Set) continue;
        if (MemEntry.MemBlock) free(MemEntry.MemBlock);
    }
}

mementry_t* GetMemEntry(uint32_t Addr)
{
    uint32_t RealAddress = Addr & 0x1FFFFFFF;
    for (size_t i = 0; i < MEMORY_ENTRIES; ++i)
    {
        if (MemEntries[i].EndAddr < RealAddress || MemEntries[i].Base > RealAddress) continue;

        if (MemEntries[i].EndAddr >= RealAddress)
            return &MemEntries[i];
    }

    // TODO: Invoke TLB Miss

    return NULL;
}

void WriteUInt8(uint8_t Value, uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    ((uint8_t*)Entry->MemBlock)[Index] = Value;
    if (Entry->WriteCallback) Entry->WriteCallback(Value, Addr);
}

uint8_t ReadUInt8(uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;
    
    if (Entry->ReadCallback) Entry->ReadCallback(Addr);
    return ((uint8_t*)Entry->MemBlock)[Index];
}

void WriteUInt16(uint16_t Value, uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    ((uint16_t*)Entry->MemBlock)[Index] = bswap_16(Value);
    if (Entry->WriteCallback) Entry->WriteCallback(Value, Addr);
}

uint16_t ReadUInt16(uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    if (Entry->ReadCallback) Entry->ReadCallback(Addr);
    return bswap_16(((uint16_t*)Entry->MemBlock)[Index]);
}

void WriteUInt32(uint32_t Value, uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    ((uint32_t*)Entry->MemBlock)[Index] = bswap_32(Value);
    if (Entry->WriteCallback) Entry->WriteCallback(Value, Addr);
}

uint32_t ReadUInt32(uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    if (Entry->ReadCallback) Entry->ReadCallback(Addr);
    return bswap_32(((uint32_t*)Entry->MemBlock)[Index]);
}

void WriteUInt64(uint64_t Value, uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    ((uint64_t*)Entry->MemBlock)[Index] = bswap_64(Value);
    if (Entry->WriteCallback) Entry->WriteCallback(Value, Addr);
}

uint64_t ReadUInt64(uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    if (Entry->ReadCallback) Entry->ReadCallback(Addr);
    return bswap_64(((uint64_t*)Entry->MemBlock)[Index]);
}

void MemoryCopy(uint32_t Dest, uint32_t Source, size_t Length)
{
    mementry_t* DestEntry = GetMemEntry(Dest);
    mementry_t* SrcEntry  = GetMemEntry(Source);
    size_t DestIndex = (Dest & 0x1FFFFFFF) - DestEntry->Base;
    size_t SrcIndex  = (Source & 0x1FFFFFFF) - SrcEntry->Base;

    memcpy(((uint8_t*)DestEntry->MemBlock) + DestIndex, ((uint8_t*)SrcEntry->MemBlock) + SrcIndex, Length);
}