#include "mem.h"

#include "config.h"
#include "cpu.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <byteswap.h>

void PI_WR_LEN_WRITE_EVENT(uint64_t Value, uint32_t Addr)
{
    printf("PIDMA: Type: Write, DMA Length: 0x%x, Cart Address: 0x%x, DRAM Address: 0x%x\n", (uint32_t)Value + 1, bswap_32(PI_CART_ADDR_REG_RW), bswap_32(PI_DRAM_ADDR_REG_RW));
    MemoryCopy(bswap_32(PI_DRAM_ADDR_REG_RW), bswap_32(PI_CART_ADDR_REG_RW), (uint32_t)Value + 1);
}

void MemoryInit(void* ROM, size_t ROMSize)
{
    size_t i = 0;

    size_t RDRAM_Size = (Config.ExpansionPak) ? 8388608 : 4194304;

    RDRAM_RW = malloc(RDRAM_Size);
    MemEntries[i].Base          = 0x00000000;
    MemEntries[i].EndAddr       = 0x00000000 + (RDRAM_Size - 1);
    MemEntries[i].MemBlockRead  = RDRAM_RW;
    MemEntries[i].MemBlockWrite = RDRAM_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = true;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    SP_DMEM_RW = malloc(0x1000);
    MemEntries[i].Base          = 0x04000000;
    MemEntries[i].EndAddr       = 0x04000FFF;
    MemEntries[i].MemBlockRead  = SP_DMEM_RW;
    MemEntries[i].MemBlockWrite = SP_DMEM_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = true;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    SP_IMEM_RW = malloc(0x1000);
    MemEntries[i].Base          = 0x04001000;
    MemEntries[i].EndAddr       = 0x04001FFF;
    MemEntries[i].MemBlockRead  = SP_IMEM_RW;
    MemEntries[i].MemBlockWrite = SP_IMEM_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = true;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x04040010;
    MemEntries[i].EndAddr       = 0x04040013;
    MemEntries[i].MemBlockRead  = &SP_STATUS_REG_R;
    MemEntries[i].MemBlockWrite = &SP_STATUS_REG_W;
    MemEntries[i].RW            = false;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x04080000;
    MemEntries[i].EndAddr       = 0x04080003;
    MemEntries[i].MemBlockRead  = &SP_PC_REG_RW;
    MemEntries[i].MemBlockWrite = &SP_PC_REG_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x04300000;
    MemEntries[i].EndAddr       = 0x04300003;
    MemEntries[i].MemBlockRead  = &MI_INIT_MODE_REG_R;
    MemEntries[i].MemBlockWrite = &MI_INIT_MODE_REG_W;
    MemEntries[i].RW            = false;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x0430000C;
    MemEntries[i].EndAddr       = 0x0430000F;
    MemEntries[i].MemBlockRead  = &MI_INTR_MASK_REG_R;
    MemEntries[i].MemBlockWrite = &MI_INTR_MASK_REG_W;
    MemEntries[i].RW            = false;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x04400000;
    MemEntries[i].EndAddr       = 0x04400003;
    MemEntries[i].MemBlockRead  = &VI_STATUS_REG_RW;
    MemEntries[i].MemBlockWrite = &VI_STATUS_REG_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x0450000C;
    MemEntries[i].EndAddr       = 0x0450000F;
    MemEntries[i].MemBlockRead  = &AI_STATUS_REG_R;
    MemEntries[i].MemBlockWrite = &AI_STATUS_REG_W;
    MemEntries[i].RW            = false;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x04600000;
    MemEntries[i].EndAddr       = 0x04600003;
    MemEntries[i].MemBlockRead  = &PI_DRAM_ADDR_REG_RW;
    MemEntries[i].MemBlockWrite = &PI_DRAM_ADDR_REG_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x04600004;
    MemEntries[i].EndAddr       = 0x04600007;
    MemEntries[i].MemBlockRead  = &PI_CART_ADDR_REG_RW;
    MemEntries[i].MemBlockWrite = &PI_CART_ADDR_REG_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x04600008;
    MemEntries[i].EndAddr       = 0x0460000B;
    MemEntries[i].MemBlockRead  = &PI_RD_LEN_REG_RW;
    MemEntries[i].MemBlockWrite = &PI_RD_LEN_REG_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x0460000C;
    MemEntries[i].EndAddr       = 0x0460000F;
    MemEntries[i].MemBlockRead  = &PI_WR_LEN_REG_RW;
    MemEntries[i].MemBlockWrite = &PI_WR_LEN_REG_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = &PI_WR_LEN_WRITE_EVENT;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x04600010;
    MemEntries[i].EndAddr       = 0x04600013;
    MemEntries[i].MemBlockRead  = &PI_STATUS_REG_RW;
    MemEntries[i].MemBlockWrite = &PI_STATUS_REG_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x0470000C;
    MemEntries[i].EndAddr       = 0x0470000F;
    MemEntries[i].MemBlockRead  = &RI_SELECT_REG_RW;
    MemEntries[i].MemBlockWrite = &RI_SELECT_REG_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x04800018;
    MemEntries[i].EndAddr       = 0x0480001B;
    MemEntries[i].MemBlockRead  = &SI_STATUS_REG_R;
    MemEntries[i].MemBlockWrite = &SI_STATUS_REG_W;
    MemEntries[i].RW            = false;
    MemEntries[i].ShouldFree    = false;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    MemEntries[i].Base          = 0x10000000;
    MemEntries[i].EndAddr       = 0x10000000 + (ROMSize - 1);
    MemEntries[i].MemBlockRead  = ROM;
    MemEntries[i].MemBlockWrite = ROM;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = true;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    PIF_RAM_RW = malloc(63);
    MemEntries[i].Base          = 0x1FC007C0;
    MemEntries[i].EndAddr       = 0x1FC007FF;
    MemEntries[i].MemBlockRead  = PIF_RAM_RW;
    MemEntries[i].MemBlockWrite = PIF_RAM_RW;
    MemEntries[i].RW            = true;
    MemEntries[i].ShouldFree    = true;
    MemEntries[i].WriteCallback = NULL;
    MemEntries[i].ReadCallback  = NULL;
    MemEntries[i].Set = true;
    ++i;

    PI_DRAM_ADDR_REG_RW = 0;
    PI_CART_ADDR_REG_RW = 0;
    PI_RD_LEN_REG_RW    = 0;
    PI_WR_LEN_REG_RW    = 0;
    PI_STATUS_REG_RW    = 0;
}

void MemoryDeInit(void)
{
    for (size_t i = 0; i < MEMORY_ENTRIES; ++i)
    {
        mementry_t MemEntry = MemEntries[i];
        if (!MemEntry.Set || !MemEntry.ShouldFree) continue;
        if (MemEntry.MemBlockRead) free(MemEntry.MemBlockRead);
        if (!MemEntry.RW && MemEntry.MemBlockWrite) free(MemEntry.MemBlockWrite);
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

static inline void NoEntryError(uint32_t Addr, bool Store)
{
    fprintf(stderr, "ERROR: Unmapped Memory Address at 0x%x!  PC: 0x%x, Read/Write: %s\n", Addr, (uint32_t)Regs.PC.Value, Store ? "Write" : "Read");
    IsRunning = false;
}

void WriteUInt8(uint8_t Value, uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    if (!Entry) 
    {
        NoEntryError(Addr, true);
        return;
    }
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    ((uint8_t*)Entry->MemBlockWrite)[Index] = Value;
    if (Entry->WriteCallback) Entry->WriteCallback(Value, Addr);
}

uint8_t ReadUInt8(uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    if (!Entry) 
    {
        NoEntryError(Addr, false);
        return 0;
    }
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;
    
    if (Entry->ReadCallback) Entry->ReadCallback(Addr);
    return ((uint8_t*)Entry->MemBlockRead)[Index];
}

void WriteUInt16(uint16_t Value, uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    if (!Entry) 
    {
        NoEntryError(Addr, true);
        return;
    }
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    *(uint16_t*)(((uint8_t*)Entry->MemBlockWrite) + Index) = bswap_16(Value);
    if (Entry->WriteCallback) Entry->WriteCallback(Value, Addr);
}

uint16_t ReadUInt16(uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    if (!Entry) 
    {
        NoEntryError(Addr, false);
        return 0;
    }
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    if (Entry->ReadCallback) Entry->ReadCallback(Addr);
    return bswap_16(*(uint16_t*)(((uint8_t*)Entry->MemBlockRead) + Index));
}

void WriteUInt32(uint32_t Value, uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    if (!Entry) 
    {
        NoEntryError(Addr, true);
        return;
    }
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    *(uint32_t*)(((uint8_t*)Entry->MemBlockWrite) + Index) = bswap_32(Value);
    if (Entry->WriteCallback) Entry->WriteCallback(Value, Addr);
}

uint32_t ReadUInt32(uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    if (!Entry) 
    {
        NoEntryError(Addr, false);
        return 0;
    }
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    if (Entry->ReadCallback) Entry->ReadCallback(Addr);
    return bswap_32(*(uint32_t*)(((uint8_t*)Entry->MemBlockRead) + Index));
}

void WriteUInt64(uint64_t Value, uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    if (!Entry) 
    {
        NoEntryError(Addr, true);
        return;
    }
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    *(uint64_t*)(((uint8_t*)Entry->MemBlockWrite) + Index) = bswap_64(Value);
    if (Entry->WriteCallback) Entry->WriteCallback(Value, Addr);
}

uint64_t ReadUInt64(uint32_t Addr)
{
    mementry_t* Entry = GetMemEntry(Addr);
    if (!Entry) 
    {
        NoEntryError(Addr, false);
        return 0;
    }
    size_t Index = (Addr & 0x1FFFFFFF) - Entry->Base;

    if (Entry->ReadCallback) Entry->ReadCallback(Addr);
    return bswap_64(*(uint64_t*)(((uint8_t*)Entry->MemBlockRead) + Index));
}

void MemoryCopy(uint32_t Dest, uint32_t Source, size_t Length)
{
    mementry_t* DestEntry = GetMemEntry(Dest);
    if (!DestEntry) 
    {
        NoEntryError(Dest, true);
        return;
    }
    mementry_t* SrcEntry  = GetMemEntry(Source);
    if (!SrcEntry) 
    {
        NoEntryError(Source, false);
        return;
    }
    size_t DestIndex = (Dest & 0x1FFFFFFF) - DestEntry->Base;
    size_t SrcIndex  = (Source & 0x1FFFFFFF) - SrcEntry->Base;

    memcpy(((uint8_t*)DestEntry->MemBlockWrite) + DestIndex, ((uint8_t*)SrcEntry->MemBlockRead) + SrcIndex, Length);
}