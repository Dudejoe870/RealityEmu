#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    bool set;

    uint32_t base;
    uint32_t end_addr;
    void*    mem_block_read;
    void*    mem_block_write;
    
    bool RW :1; // If set, it only frees mem_block_read (Use this if both mem_block_read and write are the same)
    bool should_free :1; // If set it frees the mem_block pointers at cleanup.

    void (*read_callback)(uint32_t addr); // Can be NULL
    void (*write_callback)(uint64_t value, uint32_t addr); // Can be NULL
} mementry_t;

#define MEMORY_ENTRIES 256

mementry_t mem_entries[MEMORY_ENTRIES];

void* RDRAM_RW;

void*    SP_DMEM_RW;
void*    SP_IMEM_RW;
uint32_t SP_STATUS_REG_W;
uint32_t SP_STATUS_REG_R;
uint32_t SP_PC_REG_RW;

uint32_t DPC_START_REG_RW;
uint32_t DPC_END_REG_RW;
uint32_t DPC_CURRENT_REG_R;
uint32_t DPC_CURRENT_REG_W;
uint32_t DPC_STATUS_REG_R;
uint32_t DPC_STATUS_REG_W;

uint32_t MI_INIT_MODE_REG_W;
uint32_t MI_INIT_MODE_REG_R;
uint32_t MI_INTR_MASK_REG_W;
uint32_t MI_INTR_MASK_REG_R;
uint32_t MI_INTR_REG_W;
uint32_t MI_INTR_REG_R;

uint32_t VI_STATUS_REG_RW;
uint32_t VI_ORIGIN_REG_RW;
uint32_t VI_WIDTH_REG_RW;
uint32_t VI_INTR_REG_RW;
uint32_t VI_CURRENT_REG_R;
uint32_t VI_CURRENT_REG_W;
uint32_t VI_BURST_REG_RW;
uint32_t VI_V_SYNC_REG_RW;
uint32_t VI_H_SYNC_REG_RW;
uint32_t VI_LEAP_REG_RW;
uint32_t VI_H_START_REG_RW;
uint32_t VI_V_START_REG_RW;
uint32_t VI_V_BURST_REG_RW;
uint32_t VI_X_SCALE_REG_RW;
uint32_t VI_Y_SCALE_REG_RW;

uint32_t AI_DRAM_ADDR_REG_R;
uint32_t AI_DRAM_ADDR_REG_W;
uint32_t AI_LEN_REG_RW;
uint32_t AI_CONTROL_REG_R;
uint32_t AI_CONTROL_REG_W;
uint32_t AI_STATUS_REG_R;
uint32_t AI_STATUS_REG_W;
uint32_t AI_DACRATE_REG_R;
uint32_t AI_DACRATE_REG_W;
uint32_t AI_BITRATE_REG_R;
uint32_t AI_BITRATE_REG_W;

uint32_t PI_DRAM_ADDR_REG_RW;
uint32_t PI_CART_ADDR_REG_RW;
uint32_t PI_RD_LEN_REG_RW;
uint32_t PI_WR_LEN_REG_RW;
uint32_t PI_STATUS_REG_R;
uint32_t PI_STATUS_REG_W;
uint32_t PI_BSD_DOM1_LAT_REG_RW;
uint32_t PI_BSD_DOM1_PWD_REG_RW;
uint32_t PI_BSD_DOM1_PGS_REG_RW;
uint32_t PI_BSD_DOM1_RLS_REG_RW;
uint32_t PI_BSD_DOM2_LAT_REG_RW;
uint32_t PI_BSD_DOM2_PWD_REG_RW;
uint32_t PI_BSD_DOM2_PGS_REG_RW;
uint32_t PI_BSD_DOM2_RLS_REG_RW;

uint32_t RI_SELECT_REG_RW;

uint32_t SI_STATUS_REG_W;
uint32_t SI_STATUS_REG_R;

void* PIF_RAM_RW;

void memory_init(void* ROM, size_t ROM_size);
void memory_cleanup(void);

void write_uint8(uint8_t value, uint32_t addr);
uint8_t read_uint8(uint32_t addr);

void write_uint16(uint16_t value, uint32_t addr);
uint16_t read_uint16(uint32_t addr);

void write_uint32(uint32_t value, uint32_t addr);
uint32_t read_uint32(uint32_t addr);

void write_uint64(uint64_t value, uint32_t addr);
uint64_t read_uint64(uint32_t addr);

void memory_memcpy(uint32_t dest, uint32_t source, size_t length);

void* get_real_memory_loc(uint32_t addr);
void* get_framebuffer(void);