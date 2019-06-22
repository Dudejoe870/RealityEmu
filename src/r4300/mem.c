#include "mem.h"

#include "../config.h"
#include "cpu.h"
#include "exception.h"
#include "tlb.h"
#include "../rdp/rdp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <byteswap.h>

void DPC_END_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    DPC_CURRENT_REG_R = DPC_START_REG_RW;
    RDP_wake_up();
}

void PI_WR_LEN_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    memory_memcpy(bswap_32(PI_DRAM_ADDR_REG_RW), bswap_32(PI_CART_ADDR_REG_RW), (uint32_t)value + 1);

    PI_STATUS_REG_R &= bswap_32(0b1110); // Clear DMA Busy

    printf("PIDMA: Type: Write, DMA length: 0x%x, Cart Address: 0x%x, DRAM Address: 0x%x\n", (uint32_t)value + 1, bswap_32(PI_CART_ADDR_REG_RW), bswap_32(PI_DRAM_ADDR_REG_RW));
}

void MI_INTR_MASK_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    // SP
    if ((value & 0x0001) > 0) // Clear SP
    {
        MI_INTR_MASK_REG_R &= ~(bswap_32(0x01));
    }
    else if ((value & 0x0002) > 0) // set SP
    {
        MI_INTR_MASK_REG_R |= bswap_32(0x01);
    }

    // SI
    if ((value & 0x0004) > 0) // Clear SI
    {
        MI_INTR_MASK_REG_R &= ~(bswap_32(0x02));
    }
    else if ((value & 0x0008) > 0) // set SI
    {
        MI_INTR_MASK_REG_R |= bswap_32(0x02);
    }

    // AI
    if ((value & 0x0010) > 0) // Clear AI
    {
        MI_INTR_MASK_REG_R &= ~(bswap_32(0x04));
    }
    else if ((value & 0x0020) > 0) // set AI
    {
        MI_INTR_MASK_REG_R |= bswap_32(0x04);
    }

    // VI
    if ((value & 0x0040) > 0) // Clear VI
    {
        MI_INTR_MASK_REG_R &= ~(bswap_32(0x08));
    }
    else if ((value & 0x0080) > 0) // set VI
    {
        MI_INTR_MASK_REG_R |= bswap_32(0x08);
    }

    // PI
    if ((value & 0x0100) > 0) // Clear PI
    {
        MI_INTR_MASK_REG_R &= ~(bswap_32(0x10));
    }
    else if ((value & 0x0200) > 0) // set PI
    {
        MI_INTR_MASK_REG_R |= bswap_32(0x10);
    }

    // DP
    if ((value & 0x0400) > 0) // Clear DP
    {
        MI_INTR_MASK_REG_R &= ~(bswap_32(0x20));
    }
    else if ((value & 0x0800) > 0) // set DP
    {
        MI_INTR_MASK_REG_R |= bswap_32(0x20);
    }

    MI_INTR_REG_W = 0;
}

void VI_CURRENT_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    MI_INTR_REG_R &= ~(bswap_32(0x08)); // Clear the VI Interrupt
    VI_CURRENT_REG_W = 0;
}

void memory_init(void* ROM, size_t ROM_size)
{
    size_t i = 0;

    size_t RDRAM_Size = (config.expansion_pak) ? 8388608 : 4194304;

    // Make sure these are always from the lowest Memory address up to the highest, in order!
    RDRAM_RW = malloc(RDRAM_Size);
    mem_entries[i].base          = 0x00000000;
    mem_entries[i].end_addr       = 0x00000000 + (RDRAM_Size - 1);
    mem_entries[i].mem_block_read  = RDRAM_RW;
    mem_entries[i].mem_block_write = RDRAM_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = true;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    SP_DMEM_RW = malloc(0x1000);
    mem_entries[i].base          = 0x04000000;
    mem_entries[i].end_addr       = 0x04000FFF;
    mem_entries[i].mem_block_read  = SP_DMEM_RW;
    mem_entries[i].mem_block_write = SP_DMEM_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = true;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    SP_IMEM_RW = malloc(0x1000);
    mem_entries[i].base          = 0x04001000;
    mem_entries[i].end_addr       = 0x04001FFF;
    mem_entries[i].mem_block_read  = SP_IMEM_RW;
    mem_entries[i].mem_block_write = SP_IMEM_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = true;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04040010;
    mem_entries[i].end_addr       = 0x04040013;
    mem_entries[i].mem_block_read  = &SP_STATUS_REG_R;
    mem_entries[i].mem_block_write = &SP_STATUS_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04080000;
    mem_entries[i].end_addr       = 0x04080003;
    mem_entries[i].mem_block_read  = &SP_PC_REG_RW;
    mem_entries[i].mem_block_write = &SP_PC_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04100000;
    mem_entries[i].end_addr       = 0x04100003;
    mem_entries[i].mem_block_read  = &DPC_START_REG_RW;
    mem_entries[i].mem_block_write = &DPC_START_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04100004;
    mem_entries[i].end_addr       = 0x04100007;
    mem_entries[i].mem_block_read  = &DPC_END_REG_RW;
    mem_entries[i].mem_block_write = &DPC_END_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = DPC_END_REG_WRITE_EVENT;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04100008;
    mem_entries[i].end_addr       = 0x0410000B;
    mem_entries[i].mem_block_read  = &DPC_CURRENT_REG_R;
    mem_entries[i].mem_block_write = &DPC_CURRENT_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0410000C;
    mem_entries[i].end_addr       = 0x0410000F;
    mem_entries[i].mem_block_read  = &DPC_STATUS_REG_R;
    mem_entries[i].mem_block_write = &DPC_STATUS_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04300000;
    mem_entries[i].end_addr       = 0x04300003;
    mem_entries[i].mem_block_read  = &MI_INIT_MODE_REG_R;
    mem_entries[i].mem_block_write = &MI_INIT_MODE_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04300008;
    mem_entries[i].end_addr       = 0x0430000B;
    mem_entries[i].mem_block_read  = &MI_INTR_REG_R;
    mem_entries[i].mem_block_write = &MI_INTR_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0430000C;
    mem_entries[i].end_addr       = 0x0430000F;
    mem_entries[i].mem_block_read  = &MI_INTR_MASK_REG_R;
    mem_entries[i].mem_block_write = &MI_INTR_MASK_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = MI_INTR_MASK_REG_WRITE_EVENT;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400000;
    mem_entries[i].end_addr       = 0x04400003;
    mem_entries[i].mem_block_read  = &VI_STATUS_REG_RW;
    mem_entries[i].mem_block_write = &VI_STATUS_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400004;
    mem_entries[i].end_addr       = 0x04400007;
    mem_entries[i].mem_block_read  = &VI_ORIGIN_REG_RW;
    mem_entries[i].mem_block_write = &VI_ORIGIN_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400008;
    mem_entries[i].end_addr       = 0x0440000B;
    mem_entries[i].mem_block_read  = &VI_WIDTH_REG_RW;
    mem_entries[i].mem_block_write = &VI_WIDTH_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0440000C;
    mem_entries[i].end_addr       = 0x0440000F;
    mem_entries[i].mem_block_read  = &VI_INTR_REG_RW;
    mem_entries[i].mem_block_write = &VI_INTR_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400010;
    mem_entries[i].end_addr       = 0x04400013;
    mem_entries[i].mem_block_read  = &VI_CURRENT_REG_R;
    mem_entries[i].mem_block_write = &VI_CURRENT_REG_W;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = VI_CURRENT_REG_WRITE_EVENT;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400014;
    mem_entries[i].end_addr       = 0x04400017;
    mem_entries[i].mem_block_read  = &VI_BURST_REG_RW;
    mem_entries[i].mem_block_write = &VI_BURST_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400018;
    mem_entries[i].end_addr       = 0x0440001B;
    mem_entries[i].mem_block_read  = &VI_V_SYNC_REG_RW;
    mem_entries[i].mem_block_write = &VI_V_SYNC_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0440001C;
    mem_entries[i].end_addr       = 0x0440001F;
    mem_entries[i].mem_block_read  = &VI_H_SYNC_REG_RW;
    mem_entries[i].mem_block_write = &VI_H_SYNC_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400020;
    mem_entries[i].end_addr       = 0x04400023;
    mem_entries[i].mem_block_read  = &VI_LEAP_REG_RW;
    mem_entries[i].mem_block_write = &VI_LEAP_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400024;
    mem_entries[i].end_addr       = 0x04400027;
    mem_entries[i].mem_block_read  = &VI_H_START_REG_RW;
    mem_entries[i].mem_block_write = &VI_H_START_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400028;
    mem_entries[i].end_addr       = 0x0440002B;
    mem_entries[i].mem_block_read  = &VI_V_START_REG_RW;
    mem_entries[i].mem_block_write = &VI_V_START_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0440002C;
    mem_entries[i].end_addr       = 0x0440002F;
    mem_entries[i].mem_block_read  = &VI_V_BURST_REG_RW;
    mem_entries[i].mem_block_write = &VI_V_BURST_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400030;
    mem_entries[i].end_addr       = 0x04400033;
    mem_entries[i].mem_block_read  = &VI_X_SCALE_REG_RW;
    mem_entries[i].mem_block_write = &VI_X_SCALE_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04400034;
    mem_entries[i].end_addr       = 0x04400037;
    mem_entries[i].mem_block_read  = &VI_Y_SCALE_REG_RW;
    mem_entries[i].mem_block_write = &VI_Y_SCALE_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04500000;
    mem_entries[i].end_addr       = 0x04500003;
    mem_entries[i].mem_block_read  = &AI_DRAM_ADDR_REG_R;
    mem_entries[i].mem_block_write = &AI_DRAM_ADDR_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04500004;
    mem_entries[i].end_addr       = 0x04500007;
    mem_entries[i].mem_block_read  = &AI_LEN_REG_RW;
    mem_entries[i].mem_block_write = &AI_LEN_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04500008;
    mem_entries[i].end_addr       = 0x0450000B;
    mem_entries[i].mem_block_read  = &AI_CONTROL_REG_R;
    mem_entries[i].mem_block_write = &AI_CONTROL_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0450000C;
    mem_entries[i].end_addr       = 0x0450000F;
    mem_entries[i].mem_block_read  = &AI_STATUS_REG_R;
    mem_entries[i].mem_block_write = &AI_STATUS_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04500010;
    mem_entries[i].end_addr       = 0x04500013;
    mem_entries[i].mem_block_read  = &AI_DACRATE_REG_R;
    mem_entries[i].mem_block_write = &AI_DACRATE_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04500014;
    mem_entries[i].end_addr       = 0x04500017;
    mem_entries[i].mem_block_read  = &AI_BITRATE_REG_R;
    mem_entries[i].mem_block_write = &AI_BITRATE_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600000;
    mem_entries[i].end_addr       = 0x04600003;
    mem_entries[i].mem_block_read  = &PI_DRAM_ADDR_REG_RW;
    mem_entries[i].mem_block_write = &PI_DRAM_ADDR_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600004;
    mem_entries[i].end_addr       = 0x04600007;
    mem_entries[i].mem_block_read  = &PI_CART_ADDR_REG_RW;
    mem_entries[i].mem_block_write = &PI_CART_ADDR_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600008;
    mem_entries[i].end_addr       = 0x0460000B;
    mem_entries[i].mem_block_read  = &PI_RD_LEN_REG_RW;
    mem_entries[i].mem_block_write = &PI_RD_LEN_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0460000C;
    mem_entries[i].end_addr       = 0x0460000F;
    mem_entries[i].mem_block_read  = &PI_WR_LEN_REG_RW;
    mem_entries[i].mem_block_write = &PI_WR_LEN_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = &PI_WR_LEN_WRITE_EVENT;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600010;
    mem_entries[i].end_addr       = 0x04600013;
    mem_entries[i].mem_block_read  = &PI_STATUS_REG_R;
    mem_entries[i].mem_block_write = &PI_STATUS_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600014;
    mem_entries[i].end_addr       = 0x04600017;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM1_LAT_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM1_LAT_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600018;
    mem_entries[i].end_addr       = 0x0460001B;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM1_PWD_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM1_PWD_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0460001C;
    mem_entries[i].end_addr       = 0x0460001F;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM1_PGS_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM1_PGS_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600020;
    mem_entries[i].end_addr       = 0x04600023;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM1_RLS_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM1_RLS_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600024;
    mem_entries[i].end_addr       = 0x04600027;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM2_LAT_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM2_LAT_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600028;
    mem_entries[i].end_addr       = 0x0460002B;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM2_PWD_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM2_PWD_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0460002C;
    mem_entries[i].end_addr       = 0x0460002F;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM2_PGS_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM2_PGS_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04600030;
    mem_entries[i].end_addr       = 0x04600033;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM2_RLS_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM2_RLS_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x0470000C;
    mem_entries[i].end_addr       = 0x0470000F;
    mem_entries[i].mem_block_read  = &RI_SELECT_REG_RW;
    mem_entries[i].mem_block_write = &RI_SELECT_REG_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x04800018;
    mem_entries[i].end_addr       = 0x0480001B;
    mem_entries[i].mem_block_read  = &SI_STATUS_REG_R;
    mem_entries[i].mem_block_write = &SI_STATUS_REG_W;
    mem_entries[i].RW            = false;
    mem_entries[i].should_free    = false;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base          = 0x10000000;
    mem_entries[i].end_addr       = 0x10000000 + (ROM_size - 1);
    mem_entries[i].mem_block_read  = ROM;
    mem_entries[i].mem_block_write = ROM;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = true;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;

    PIF_RAM_RW = malloc(64);
    mem_entries[i].base          = 0x1FC007C0;
    mem_entries[i].end_addr       = 0x1FC007FF;
    mem_entries[i].mem_block_read  = PIF_RAM_RW;
    mem_entries[i].mem_block_write = PIF_RAM_RW;
    mem_entries[i].RW            = true;
    mem_entries[i].should_free    = true;
    mem_entries[i].write_callback = NULL;
    mem_entries[i].read_callback  = NULL;
    mem_entries[i].set = true;
    ++i;
}

void memory_cleanup(void)
{
    for (size_t i = 0; i < MEMORY_ENTRIES; ++i)
    {
        mementry_t mem_entry = mem_entries[i];
        if (!mem_entry.set || !mem_entry.should_free) continue;
        if (mem_entry.mem_block_read) free(mem_entry.mem_block_read);
        if (!mem_entry.RW && mem_entry.mem_block_write) free(mem_entry.mem_block_write);
    }
}

__attribute__((__always_inline__)) static inline mementry_t* GetMementry(uint32_t addr, bool store)
{
    uint32_t real_address = addr & 0x1FFFFFFF;

    uint32_t last_base = 0;
    for (size_t i = 0; i < MEMORY_ENTRIES; ++i)
    {
        if (mem_entries[i].base    > real_address) continue;
        if (mem_entries[i].end_addr < real_address) continue;

        if (real_address < last_base) break;

        if (mem_entries[i].end_addr >= real_address)
            return &mem_entries[i];
        last_base = mem_entries[i].base;
    }

    if ((addr & 0xC0000000) != 0x80000000) invoke_TLB_miss(addr, store);

    return NULL;
}

__attribute__((__always_inline__)) static inline void NoentryError(uint32_t addr, bool store)
{
    fprintf(stderr, "ERROR: Unmapped Memory Address at 0x%x!  PC: 0x%x, Read/Write: %s\n", addr, (uint32_t)regs.PC.value, store ? "Write" : "Read");
    if ((addr & 0xC0000000) == 0x80000000) is_running = false;
}

__attribute__((__always_inline__)) static inline int GetFinalTranslation(uint32_t addr, mementry_t** entry, size_t* index, bool store)
{
    uint32_t non_cached_addr = TLB_translate_address(addr);
    *entry = GetMementry(non_cached_addr, store);
    if (!(*entry))
    {
        NoentryError(addr, true);
        return 1;
    }
    non_cached_addr &= 0x1FFFFFFF;
    *index = non_cached_addr - (*entry)->base;
    return 0;
}

void write_uint8(uint8_t value, uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (GetFinalTranslation(addr, &entry, &index, true) != 0) return;

    ((uint8_t*)entry->mem_block_write)[index] = value;
    if (entry->write_callback) entry->write_callback(value, addr);
}

uint8_t read_uint8(uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (GetFinalTranslation(addr, &entry, &index, false) != 0) return 0;
    
    if (entry->read_callback) entry->read_callback(addr);
    return ((uint8_t*)entry->mem_block_read)[index];
}

void write_uint16(uint16_t value, uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (GetFinalTranslation(addr, &entry, &index, true) != 0) return;

    *(uint16_t*)(((uint8_t*)entry->mem_block_write) + index) = bswap_16(value);
    if (entry->write_callback) entry->write_callback(value, addr);
}

uint16_t read_uint16(uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (GetFinalTranslation(addr, &entry, &index, false) != 0) return 0;

    if (entry->read_callback) entry->read_callback(addr);
    return bswap_16(*(uint16_t*)(((uint8_t*)entry->mem_block_read) + index));
}

void write_uint32(uint32_t value, uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (GetFinalTranslation(addr, &entry, &index, true) != 0) return;

    *(uint32_t*)(((uint8_t*)entry->mem_block_write) + index) = bswap_32(value);
    if (entry->write_callback) entry->write_callback(value, addr);
}

uint32_t read_uint32(uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (GetFinalTranslation(addr, &entry, &index, false) != 0) return 0;

    if (entry->read_callback) entry->read_callback(addr);
    return bswap_32(*(uint32_t*)(((uint8_t*)entry->mem_block_read) + index));
}

void write_uint64(uint64_t value, uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (GetFinalTranslation(addr, &entry, &index, true) != 0) return;

    *(uint64_t*)(((uint8_t*)entry->mem_block_write) + index) = bswap_64(value);
    if (entry->write_callback) entry->write_callback(value, addr);
}

uint64_t read_uint64(uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (GetFinalTranslation(addr, &entry, &index, false) != 0) return 0;

    if (entry->read_callback) entry->read_callback(addr);
    return bswap_64(*(uint64_t*)(((uint8_t*)entry->mem_block_read) + index));
}

void memory_memcpy(uint32_t dest, uint32_t source, size_t length)
{
    mementry_t* dst_entry;
    mementry_t* src_entry;
    size_t dst_index = 0;
    size_t src_index = 0;

    if (GetFinalTranslation(dest,   &dst_entry, &dst_index, true)  != 0) return;
    if (GetFinalTranslation(source, &src_entry, &src_index, false) != 0) return;

    memcpy(((uint8_t*)dst_entry->mem_block_write) + dst_index, ((uint8_t*)src_entry->mem_block_read) + src_index, length);
}

void* get_real_memory_loc(uint32_t addr)
{
    void* res = NULL;

    uint32_t non_cached_addr = TLB_translate_address(addr);
    mementry_t* entry = GetMementry(non_cached_addr, false);
    if (!entry)
        return NULL;
    non_cached_addr &= 0x1FFFFFFF;
    size_t index = non_cached_addr - entry->base;

    res = (void*)((uint8_t*)entry->mem_block_read + index);

    return res;
}

void* get_framebuffer(void)
{
    return get_real_memory_loc(bswap_32(VI_ORIGIN_REG_RW) | 0xA0000000);
}