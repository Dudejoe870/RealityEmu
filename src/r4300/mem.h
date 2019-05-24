#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    bool Set;

    uint32_t Base;
    uint32_t EndAddr;
    void*    MemBlock;

    void (*ReadCallback)(uint32_t Addr); // Can be NULL
    void (*WriteCallback)(uint64_t Value, uint32_t Addr); // Can be NULL
} mementry_t;

#define MEMORY_ENTRIES 32

mementry_t MemEntries[MEMORY_ENTRIES];

void MemoryInit(void* ROM, size_t ROMSize);
void MemoryDeInit(void);

void WriteUInt8(uint8_t Value, uint32_t Addr);
uint8_t ReadUInt8(uint32_t Addr);

void WriteUInt16(uint16_t Value, uint32_t Addr);
uint16_t ReadUInt16(uint32_t Addr);

void WriteUInt32(uint32_t Value, uint32_t Addr);
uint32_t ReadUInt32(uint32_t Addr);

void WriteUInt64(uint64_t Value, uint32_t Addr);
uint64_t ReadUInt64(uint32_t Addr);

void MemoryCopy(uint32_t Dest, uint32_t Source, size_t Length);