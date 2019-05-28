#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    bool Set;

    uint32_t Base;
    uint32_t EndAddr;
    void*    MemBlockRead;
    void*    MemBlockWrite;
    bool RW; // If set, it only frees MemBlockRead (Use this if both MemBlockRead and Write are the same)
    bool ShouldFree; // If set it frees the MemBlock pointer at cleanup.

    void (*ReadCallback)(uint32_t Addr); // Can be NULL
    void (*WriteCallback)(uint64_t Value, uint32_t Addr); // Can be NULL
} mementry_t;

#define MEMORY_ENTRIES 32

mementry_t MemEntries[MEMORY_ENTRIES];

void* RDRAM_RW;

void*    SP_DMEM_RW;
void*    SP_IMEM_RW;
uint32_t SP_STATUS_REG_W;
uint32_t SP_STATUS_REG_R;
uint32_t SP_PC_REG_RW;

uint32_t MI_INIT_MODE_REG_W;
uint32_t MI_INIT_MODE_REG_R;
uint32_t MI_INTR_MASK_REG_W;
uint32_t MI_INTR_MASK_REG_R;

uint32_t VI_STATUS_REG_RW;
uint32_t VI_ORIGIN_REG_RW;
uint32_t VI_WIDTH_REG_RW;
uint32_t VI_INTR_REG_RW;
uint32_t VI_CURRENT_REG_RW;
uint32_t VI_BURST_REG_RW;
uint32_t VI_V_SYNC_REG_RW;
uint32_t VI_H_SYNC_REG_RW;
uint32_t VI_LEAP_REG_RW;
uint32_t VI_H_START_REG_RW;
uint32_t VI_V_START_REG_RW;
uint32_t VI_V_BURST_REG_RW;
uint32_t VI_X_SCALE_REG_RW;
uint32_t VI_Y_SCALE_REG_RW;

uint32_t AI_STATUS_REG_R;
uint32_t AI_STATUS_REG_W;

uint32_t PI_DRAM_ADDR_REG_RW;
uint32_t PI_CART_ADDR_REG_RW;
uint32_t PI_RD_LEN_REG_RW;
uint32_t PI_WR_LEN_REG_RW;
uint32_t PI_STATUS_REG_RW;

uint32_t RI_SELECT_REG_RW;

uint32_t SI_STATUS_REG_W;
uint32_t SI_STATUS_REG_R;

void* PIF_RAM_RW;

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

void* GetFramebuffer(void);