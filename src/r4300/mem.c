#include "r4300/mem.h"

#include "common.h"

void* framebuffer_addr = NULL;

void SP_RD_LEN_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    uint16_t len   =  value & 0x00000FFF;
    uint8_t  count = (value & 0x000FF000) >> 12;
    uint16_t skip  = (value & 0xFFF00000) >> 20;

    uint8_t* MEM = (bswap_32(SP_MEM_ADDR_REG_RW) & 0x1000) ? SP_IMEM_RW : SP_DMEM_RW;

    if (config.debug_logging) 
        printf("RSPDMA: Type: Read, DMA Length: %u, DMA Count: %u, Skip: %u, RSP Address: 0x%x, DRAM Address: 0x%x, RSP Mem Type: %s\n", 
            len + 1, count + 1, skip, 
            bswap_32(SP_MEM_ADDR_REG_RW) & 0xFFF, bswap_32(SP_DRAM_ADDR_REG_RW),
            (bswap_32(SP_MEM_ADDR_REG_RW) & 0x1000) ? "IMEM" : "DMEM");

    uint32_t index = 0;
    uint32_t index_no_skip = 0;
    for (uint32_t j = 0; j <= count; ++j)
    {
        for (uint32_t i = 0; i <= len; ++i, ++index, ++index_no_skip)
        {
            MEM[(bswap_32(SP_MEM_ADDR_REG_RW) & 0xFFF) + index_no_skip] = read_uint8((bswap_32(SP_DRAM_ADDR_REG_RW) & 0x1FFFFFFF) + index);
        }
        index += skip;
    }
}

void SP_WR_LEN_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    uint16_t len   =  value & 0x00000FFF;
    uint8_t  count = (value & 0x000FF000) >> 12;
    uint16_t skip  = (value & 0xFFF00000) >> 20;

    uint8_t* MEM = (bswap_32(SP_MEM_ADDR_REG_RW) & 0x1000) ? SP_IMEM_RW : SP_DMEM_RW;

    if (config.debug_logging) 
        printf("RSPDMA: Type: Write, DMA Length: %u, DMA Count: %u, Skip: %u, RSP Address: 0x%x, DRAM Address: 0x%x, RSP Mem Type: %s\n", 
            len + 1, count + 1, skip, 
            bswap_32(SP_MEM_ADDR_REG_RW) & 0xFFF, bswap_32(SP_DRAM_ADDR_REG_RW),
            (bswap_32(SP_MEM_ADDR_REG_RW) & 0x1000) ? "IMEM" : "DMEM");

    uint32_t index = 0;
    uint32_t index_no_skip = 0;
    for (uint32_t j = 0; j <= count; ++j)
    {
        for (uint32_t i = 0; i <= len; ++i, ++index, ++index_no_skip)
        {
            write_uint8(MEM[(bswap_32(SP_MEM_ADDR_REG_RW) & 0xFFF) + index_no_skip], (bswap_32(SP_DRAM_ADDR_REG_RW) & 0x1FFFFFFF) + index);
        }
        index += skip;
    }
}

void SP_STATUS_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    if ((value & 0x0001) > 0) // Clear halt
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x01));
    }
    else if ((value & 0x0002) > 0) // Set halt
    {
        SP_STATUS_REG_R |= bswap_32(0x01);
    }

    if ((value & 0x0004) > 0) // Clear broke
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x02));
    }

    if ((value & 0x0008) > 0) // Clear intr
    {
        MI_INTR_REG_R &= ~(bswap_32(1 << MI_INTR_SP));
    }
    else if ((value & 0x0010) > 0) // Set intr
    {
        invoke_mi_interrupt(MI_INTR_SP);
    }

    if ((value & 0x0020) > 0) // Clear sstep
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x20));
    }
    else if ((value & 0x0040) > 0) // Set sstep
    {
        SP_STATUS_REG_R |= bswap_32(0x20);
    }

    if ((value & 0x0080) > 0) // Clear intr on break
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x40));
    }
    else if ((value & 0x0100) > 0) // Set intr on break
    {
        SP_STATUS_REG_R |= bswap_32(0x40);
    }

    if ((value & 0x0200) > 0) // Clear signal 0
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x80));
    }
    else if ((value & 0x0400) > 0) // Set signal 0
    {
        SP_STATUS_REG_R |= bswap_32(0x80);
    }

    if ((value & 0x0800) > 0) // Clear signal 1
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x100));
    }
    else if ((value & 0x1000) > 0) // Set signal 1
    {
        SP_STATUS_REG_R |= bswap_32(0x100);
    }

    if ((value & 0x2000) > 0) // Clear signal 2
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x200));
    }
    else if ((value & 0x4000) > 0) // Set signal 2
    {
        SP_STATUS_REG_R |= bswap_32(0x200);
    }
    
    if ((value & 0x8000) > 0) // Clear signal 3
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x400));
    }
    else if ((value & 0x10000) > 0) // Set signal 3
    {
        SP_STATUS_REG_R |= bswap_32(0x400);
    }

    if ((value & 0x20000) > 0) // Clear signal 4
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x800));
    }
    else if ((value & 0x40000) > 0) // Set signal 4
    {
        SP_STATUS_REG_R |= bswap_32(0x800);
    }

    if ((value & 0x80000) > 0) // Clear signal 5
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x1000));
    }
    else if ((value & 0x100000) > 0) // Set signal 5
    {
        SP_STATUS_REG_R |= bswap_32(0x1000);
    }

    if ((value & 0x200000) > 0) // Clear signal 6
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x2000));
    }
    else if ((value & 0x400000) > 0) // Set signal 6
    {
        SP_STATUS_REG_R |= bswap_32(0x2000);
    }

    if ((value & 0x800000) > 0) // Clear signal 7
    {
        SP_STATUS_REG_R &= ~(bswap_32(0x4000));
    }
    else if ((value & 0x1000000) > 0) // Set signal 7
    {
        SP_STATUS_REG_R |= bswap_32(0x4000);
    }

    SP_STATUS_REG_W = 0;
}

void DPC_END_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    DPC_CURRENT_REG_R = DPC_START_REG_RW;
    RDP_wake_up();
}

void PI_WR_LEN_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    if (config.debug_logging) printf("PIDMA: Type: Write, DMA Length: 0x%x, Cart Address: 0x%x, DRAM Address: 0x%x\n", (uint32_t)value + 1, bswap_32(PI_CART_ADDR_REG_RW), bswap_32(PI_DRAM_ADDR_REG_RW));
    
    memory_memcpy(bswap_32(PI_DRAM_ADDR_REG_RW), bswap_32(PI_CART_ADDR_REG_RW), (uint32_t)value + 1);

    PI_STATUS_REG_R &= bswap_32(~0b0001); // Clear DMA Busy

    invoke_mi_interrupt(MI_INTR_PI);
}

void PI_STATUS_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    if ((value & 2) > 0)
    {
        MI_INTR_REG_R &= ~(bswap_32(1 << MI_INTR_PI));
    }

    PI_STATUS_REG_W = 0;
}

void MI_INIT_MODE_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    if ((value & 0x0800) > 0)
    {
        MI_INTR_REG_R &= ~(bswap_32(1 << MI_INTR_DP));
    }

    MI_INIT_MODE_REG_W = 0;
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

void VI_ORIGIN_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    framebuffer_addr = get_real_memory_loc((uint32_t)value + 0xA0000000);
}

void VI_CURRENT_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    MI_INTR_REG_R &= ~(bswap_32(1 << MI_INTR_VI)); // Clear the VI Interrupt
    VI_CURRENT_REG_W = 0;
}

void AI_DRAM_ADDR_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    invoke_mi_interrupt(MI_INTR_AI);
}

void AI_STATUS_REG_WRITE_EVENT(uint64_t value, uint32_t addr)
{
    MI_INTR_REG_R &= ~(bswap_32(0x04)); // Clear the AI Interrupt
    AI_STATUS_REG_W = 0;
}

void PIF_RAM_READ_EVENT(uint32_t addr)
{
    if ((addr & 0x1FFFFFFF) == 0x1FC007FC) ((uint8_t*)PIF_RAM_RW)[0x3F] = 0x80;
}

void memory_init(void* ROM, size_t ROM_size)
{
    size_t i = 0;

    size_t RDRAM_Size = (config.expansion_pak) ? 8388608 : 4194304;

    // Make sure these are always from the lowest Memory address up to the highest, in order!
    RDRAM_RW = malloc(RDRAM_Size);
    mem_entries[i].base            = 0x00000000;
    mem_entries[i].end_addr        = 0x00000000 + (RDRAM_Size - 1);
    mem_entries[i].mem_block_read  = RDRAM_RW;
    mem_entries[i].mem_block_write = RDRAM_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = true;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    if (!config.expansion_pak)
    {
        mem_entries[i].base            = 0x00400000;
        mem_entries[i].end_addr        = 0x007FFFFF;
        mem_entries[i].mem_block_read  = NULL;
        mem_entries[i].mem_block_write = NULL;
        mem_entries[i].RW              = false;
        mem_entries[i].should_free     = false;
        mem_entries[i].write_callback  = NULL;
        mem_entries[i].read_callback   = NULL;
        mem_entries[i].set = true;
        ++i;
    }

    mem_entries[i].base            = 0x00800000;
    mem_entries[i].end_addr        = 0x03EFFFFF;
    mem_entries[i].mem_block_read  = NULL;
    mem_entries[i].mem_block_write = NULL;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F00000;
    mem_entries[i].end_addr        = 0x03F00003;
    mem_entries[i].mem_block_read  = &RDRAM_CONFIG_REG;
    mem_entries[i].mem_block_write = &RDRAM_CONFIG_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F00004;
    mem_entries[i].end_addr        = 0x03F00007;
    mem_entries[i].mem_block_read  = &RDRAM_DEVICE_ID_REG;
    mem_entries[i].mem_block_write = &RDRAM_DEVICE_ID_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F00008;
    mem_entries[i].end_addr        = 0x03F0000B;
    mem_entries[i].mem_block_read  = &RDRAM_DELAY_REG;
    mem_entries[i].mem_block_write = &RDRAM_DELAY_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F0000C;
    mem_entries[i].end_addr        = 0x03F0000F;
    mem_entries[i].mem_block_read  = &RDRAM_MODE_REG;
    mem_entries[i].mem_block_write = &RDRAM_MODE_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F00010;
    mem_entries[i].end_addr        = 0x03F00013;
    mem_entries[i].mem_block_read  = &RDRAM_REF_INTERVAL_REG;
    mem_entries[i].mem_block_write = &RDRAM_REF_INTERVAL_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F00014;
    mem_entries[i].end_addr        = 0x03F00017;
    mem_entries[i].mem_block_read  = &RDRAM_REF_ROW_REG;
    mem_entries[i].mem_block_write = &RDRAM_REF_ROW_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F00018;
    mem_entries[i].end_addr        = 0x03F0001B;
    mem_entries[i].mem_block_read  = &RDRAM_RAS_INTERVAL_REG;
    mem_entries[i].mem_block_write = &RDRAM_RAS_INTERVAL_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F0001C;
    mem_entries[i].end_addr        = 0x03F0001F;
    mem_entries[i].mem_block_read  = &RDRAM_MIN_INTERVAL_REG;
    mem_entries[i].mem_block_write = &RDRAM_MIN_INTERVAL_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F00020;
    mem_entries[i].end_addr        = 0x03F00023;
    mem_entries[i].mem_block_read  = &RDRAM_ADDR_SELECT_REG;
    mem_entries[i].mem_block_write = &RDRAM_ADDR_SELECT_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x03F00024;
    mem_entries[i].end_addr        = 0x03F00027;
    mem_entries[i].mem_block_read  = &RDRAM_DEVICE_MANUF_REG;
    mem_entries[i].mem_block_write = &RDRAM_DEVICE_MANUF_REG;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    SP_DMEM_RW = malloc(0x1000);
    mem_entries[i].base            = 0x04000000;
    mem_entries[i].end_addr        = 0x04000FFF;
    mem_entries[i].mem_block_read  = SP_DMEM_RW;
    mem_entries[i].mem_block_write = SP_DMEM_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = true;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    SP_IMEM_RW = malloc(0x1000);
    mem_entries[i].base            = 0x04001000;
    mem_entries[i].end_addr        = 0x04001FFF;
    mem_entries[i].mem_block_read  = SP_IMEM_RW;
    mem_entries[i].mem_block_write = SP_IMEM_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = true;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04040000;
    mem_entries[i].end_addr        = 0x04040003;
    mem_entries[i].mem_block_read  = &SP_MEM_ADDR_REG_RW;
    mem_entries[i].mem_block_write = &SP_MEM_ADDR_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04040004;
    mem_entries[i].end_addr        = 0x04040007;
    mem_entries[i].mem_block_read  = &SP_DRAM_ADDR_REG_RW;
    mem_entries[i].mem_block_write = &SP_DRAM_ADDR_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04040008;
    mem_entries[i].end_addr        = 0x0404000B;
    mem_entries[i].mem_block_read  = &SP_RD_LEN_REG_RW;
    mem_entries[i].mem_block_write = &SP_RD_LEN_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = SP_RD_LEN_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0404000C;
    mem_entries[i].end_addr        = 0x0404000F;
    mem_entries[i].mem_block_read  = &SP_WR_LEN_REG_RW;
    mem_entries[i].mem_block_write = &SP_WR_LEN_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = SP_WR_LEN_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04040010;
    mem_entries[i].end_addr        = 0x04040013;
    mem_entries[i].mem_block_read  = &SP_STATUS_REG_R;
    mem_entries[i].mem_block_write = &SP_STATUS_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = SP_STATUS_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04080000;
    mem_entries[i].end_addr        = 0x04080003;
    mem_entries[i].mem_block_read  = &SP_PC_REG_RW;
    mem_entries[i].mem_block_write = &SP_PC_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04100000;
    mem_entries[i].end_addr        = 0x04100003;
    mem_entries[i].mem_block_read  = &DPC_START_REG_RW;
    mem_entries[i].mem_block_write = &DPC_START_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04100004;
    mem_entries[i].end_addr        = 0x04100007;
    mem_entries[i].mem_block_read  = &DPC_END_REG_RW;
    mem_entries[i].mem_block_write = &DPC_END_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = DPC_END_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base             = 0x04100008;
    mem_entries[i].end_addr         = 0x0410000B;
    mem_entries[i].mem_block_read   = &DPC_CURRENT_REG_R;
    mem_entries[i].mem_block_write  = &DPC_CURRENT_REG_W;
    mem_entries[i].RW               = false;
    mem_entries[i].should_free      = false;
    mem_entries[i].write_callback   = NULL;
    mem_entries[i].read_callback    = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0410000C;
    mem_entries[i].end_addr        = 0x0410000F;
    mem_entries[i].mem_block_read  = &DPC_STATUS_REG_R;
    mem_entries[i].mem_block_write = &DPC_STATUS_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04300000;
    mem_entries[i].end_addr        = 0x04300003;
    mem_entries[i].mem_block_read  = &MI_INIT_MODE_REG_R;
    mem_entries[i].mem_block_write = &MI_INIT_MODE_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = MI_INIT_MODE_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04300008;
    mem_entries[i].end_addr        = 0x0430000B;
    mem_entries[i].mem_block_read  = &MI_INTR_REG_R;
    mem_entries[i].mem_block_write = &MI_INTR_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0430000C;
    mem_entries[i].end_addr        = 0x0430000F;
    mem_entries[i].mem_block_read  = &MI_INTR_MASK_REG_R;
    mem_entries[i].mem_block_write = &MI_INTR_MASK_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = MI_INTR_MASK_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400000;
    mem_entries[i].end_addr        = 0x04400003;
    mem_entries[i].mem_block_read  = &VI_STATUS_REG_RW;
    mem_entries[i].mem_block_write = &VI_STATUS_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400004;
    mem_entries[i].end_addr        = 0x04400007;
    mem_entries[i].mem_block_read  = &VI_ORIGIN_REG_RW;
    mem_entries[i].mem_block_write = &VI_ORIGIN_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = VI_ORIGIN_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400008;
    mem_entries[i].end_addr        = 0x0440000B;
    mem_entries[i].mem_block_read  = &VI_WIDTH_REG_RW;
    mem_entries[i].mem_block_write = &VI_WIDTH_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0440000C;
    mem_entries[i].end_addr        = 0x0440000F;
    mem_entries[i].mem_block_read  = &VI_INTR_REG_RW;
    mem_entries[i].mem_block_write = &VI_INTR_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400010;
    mem_entries[i].end_addr        = 0x04400013;
    mem_entries[i].mem_block_read  = &VI_CURRENT_REG_R;
    mem_entries[i].mem_block_write = &VI_CURRENT_REG_W;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = VI_CURRENT_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400014;
    mem_entries[i].end_addr        = 0x04400017;
    mem_entries[i].mem_block_read  = &VI_BURST_REG_RW;
    mem_entries[i].mem_block_write = &VI_BURST_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400018;
    mem_entries[i].end_addr        = 0x0440001B;
    mem_entries[i].mem_block_read  = &VI_V_SYNC_REG_RW;
    mem_entries[i].mem_block_write = &VI_V_SYNC_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0440001C;
    mem_entries[i].end_addr        = 0x0440001F;
    mem_entries[i].mem_block_read  = &VI_H_SYNC_REG_RW;
    mem_entries[i].mem_block_write = &VI_H_SYNC_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400020;
    mem_entries[i].end_addr        = 0x04400023;
    mem_entries[i].mem_block_read  = &VI_LEAP_REG_RW;
    mem_entries[i].mem_block_write = &VI_LEAP_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400024;
    mem_entries[i].end_addr        = 0x04400027;
    mem_entries[i].mem_block_read  = &VI_H_START_REG_RW;
    mem_entries[i].mem_block_write = &VI_H_START_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400028;
    mem_entries[i].end_addr        = 0x0440002B;
    mem_entries[i].mem_block_read  = &VI_V_START_REG_RW;
    mem_entries[i].mem_block_write = &VI_V_START_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0440002C;
    mem_entries[i].end_addr        = 0x0440002F;
    mem_entries[i].mem_block_read  = &VI_V_BURST_REG_RW;
    mem_entries[i].mem_block_write = &VI_V_BURST_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400030;
    mem_entries[i].end_addr        = 0x04400033;
    mem_entries[i].mem_block_read  = &VI_X_SCALE_REG_RW;
    mem_entries[i].mem_block_write = &VI_X_SCALE_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04400034;
    mem_entries[i].end_addr        = 0x04400037;
    mem_entries[i].mem_block_read  = &VI_Y_SCALE_REG_RW;
    mem_entries[i].mem_block_write = &VI_Y_SCALE_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04500000;
    mem_entries[i].end_addr        = 0x04500003;
    mem_entries[i].mem_block_read  = &AI_DRAM_ADDR_REG_R;
    mem_entries[i].mem_block_write = &AI_DRAM_ADDR_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = AI_DRAM_ADDR_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04500004;
    mem_entries[i].end_addr        = 0x04500007;
    mem_entries[i].mem_block_read  = &AI_LEN_REG_RW;
    mem_entries[i].mem_block_write = &AI_LEN_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04500008;
    mem_entries[i].end_addr        = 0x0450000B;
    mem_entries[i].mem_block_read  = &AI_CONTROL_REG_R;
    mem_entries[i].mem_block_write = &AI_CONTROL_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0450000C;
    mem_entries[i].end_addr        = 0x0450000F;
    mem_entries[i].mem_block_read  = &AI_STATUS_REG_R;
    mem_entries[i].mem_block_write = &AI_STATUS_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = AI_STATUS_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04500010;
    mem_entries[i].end_addr        = 0x04500013;
    mem_entries[i].mem_block_read  = &AI_DACRATE_REG_R;
    mem_entries[i].mem_block_write = &AI_DACRATE_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04500014;
    mem_entries[i].end_addr        = 0x04500017;
    mem_entries[i].mem_block_read  = &AI_BITRATE_REG_R;
    mem_entries[i].mem_block_write = &AI_BITRATE_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600000;
    mem_entries[i].end_addr        = 0x04600003;
    mem_entries[i].mem_block_read  = &PI_DRAM_ADDR_REG_RW;
    mem_entries[i].mem_block_write = &PI_DRAM_ADDR_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600004;
    mem_entries[i].end_addr        = 0x04600007;
    mem_entries[i].mem_block_read  = &PI_CART_ADDR_REG_RW;
    mem_entries[i].mem_block_write = &PI_CART_ADDR_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600008;
    mem_entries[i].end_addr        = 0x0460000B;
    mem_entries[i].mem_block_read  = &PI_RD_LEN_REG_RW;
    mem_entries[i].mem_block_write = &PI_RD_LEN_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0460000C;
    mem_entries[i].end_addr        = 0x0460000F;
    mem_entries[i].mem_block_read  = &PI_WR_LEN_REG_RW;
    mem_entries[i].mem_block_write = &PI_WR_LEN_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = &PI_WR_LEN_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600010;
    mem_entries[i].end_addr        = 0x04600013;
    mem_entries[i].mem_block_read  = &PI_STATUS_REG_R;
    mem_entries[i].mem_block_write = &PI_STATUS_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = PI_STATUS_REG_WRITE_EVENT;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600014;
    mem_entries[i].end_addr        = 0x04600017;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM1_LAT_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM1_LAT_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600018;
    mem_entries[i].end_addr        = 0x0460001B;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM1_PWD_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM1_PWD_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0460001C;
    mem_entries[i].end_addr        = 0x0460001F;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM1_PGS_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM1_PGS_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600020;
    mem_entries[i].end_addr        = 0x04600023;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM1_RLS_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM1_RLS_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600024;
    mem_entries[i].end_addr        = 0x04600027;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM2_LAT_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM2_LAT_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600028;
    mem_entries[i].end_addr        = 0x0460002B;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM2_PWD_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM2_PWD_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0460002C;
    mem_entries[i].end_addr        = 0x0460002F;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM2_PGS_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM2_PGS_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04600030;
    mem_entries[i].end_addr        = 0x04600033;
    mem_entries[i].mem_block_read  = &PI_BSD_DOM2_RLS_REG_RW;
    mem_entries[i].mem_block_write = &PI_BSD_DOM2_RLS_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x0470000C;
    mem_entries[i].end_addr        = 0x0470000F;
    mem_entries[i].mem_block_read  = &RI_SELECT_REG_RW;
    mem_entries[i].mem_block_write = &RI_SELECT_REG_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x04800018;
    mem_entries[i].end_addr        = 0x0480001B;
    mem_entries[i].mem_block_read  = &SI_STATUS_REG_R;
    mem_entries[i].mem_block_write = &SI_STATUS_REG_W;
    mem_entries[i].RW              = false;
    mem_entries[i].should_free     = false;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    mem_entries[i].base            = 0x10000000;
    mem_entries[i].end_addr        = 0x10000000 + (ROM_size - 1);
    mem_entries[i].mem_block_read  = ROM;
    mem_entries[i].mem_block_write = ROM;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = true;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = NULL;
    mem_entries[i].set = true;
    ++i;

    PIF_RAM_RW = malloc(64);
    mem_entries[i].base            = 0x1FC007C0;
    mem_entries[i].end_addr        = 0x1FC007FF;
    mem_entries[i].mem_block_read  = PIF_RAM_RW;
    mem_entries[i].mem_block_write = PIF_RAM_RW;
    mem_entries[i].RW              = true;
    mem_entries[i].should_free     = true;
    mem_entries[i].write_callback  = NULL;
    mem_entries[i].read_callback   = PIF_RAM_READ_EVENT;
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

__attribute__((__always_inline__)) static inline mementry_t* get_mementry(uint32_t addr, bool store)
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

__attribute__((__always_inline__)) static inline void no_entry_error(uint32_t addr, bool store)
{
    fprintf(stderr, "ERROR: Unmapped Memory Address at 0x%x!  PC: 0x%x, Read/Write: %s\n", addr, (uint32_t)r4300.regs.PC.value, store ? "Write" : "Read");
    if ((addr & 0xC0000000) == 0x80000000) is_running = false;
}

__attribute__((__always_inline__)) static inline int get_final_translation(uint32_t addr, mementry_t** entry, size_t* index, bool store)
{
    uint32_t non_cached_addr = TLB_translate_address(addr);
    *entry = get_mementry(non_cached_addr, store);
    if (!(*entry))
    {
        no_entry_error(addr, true);
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
    if (get_final_translation(addr, &entry, &index, true) != 0) return;

    if (entry->mem_block_write)
    {
        ((uint8_t*)entry->mem_block_write)[index] = value;
        if (entry->write_callback) entry->write_callback(value, addr);
    }
}

uint8_t read_uint8(uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (get_final_translation(addr, &entry, &index, false) != 0) return 0;
    
    if (entry->mem_block_read)
    {
        if (entry->read_callback) entry->read_callback(addr);
        return ((uint8_t*)entry->mem_block_read)[index];
    }
    
    return 0;
}

void write_uint16(uint16_t value, uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (get_final_translation(addr, &entry, &index, true) != 0) return;

    if (entry->mem_block_write)
    {
        *(uint16_t*)(((uint8_t*)entry->mem_block_write) + index) = bswap_16(value);
        if (entry->write_callback) entry->write_callback(value, addr);
    }
}

uint16_t read_uint16(uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (get_final_translation(addr, &entry, &index, false) != 0) return 0;

    if (entry->mem_block_read)
    {
        if (entry->read_callback) entry->read_callback(addr);
        return bswap_16(*(uint16_t*)(((uint8_t*)entry->mem_block_read) + index));
    }

    return 0;
}

void write_uint32(uint32_t value, uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (get_final_translation(addr, &entry, &index, true) != 0) return;

    if (entry->mem_block_write)
    {
        *(uint32_t*)(((uint8_t*)entry->mem_block_write) + index) = bswap_32(value);
        if (entry->write_callback) entry->write_callback(value, addr);
    }
}

uint32_t read_uint32(uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (get_final_translation(addr, &entry, &index, false) != 0) return 0;

    if (entry->mem_block_read)
    {
        if (entry->read_callback) entry->read_callback(addr);
        return bswap_32(*(uint32_t*)(((uint8_t*)entry->mem_block_read) + index));
    }

    return 0;
}

void write_uint64(uint64_t value, uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (get_final_translation(addr, &entry, &index, true) != 0) return;

    if (entry->mem_block_write)
    {
        *(uint64_t*)(((uint8_t*)entry->mem_block_write) + index) = bswap_64(value);
        if (entry->write_callback) entry->write_callback(value, addr);
    }
}

uint64_t read_uint64(uint32_t addr)
{
    mementry_t* entry;
    size_t index;
    if (get_final_translation(addr, &entry, &index, false) != 0) return 0;

    if (entry->mem_block_read)
    {
        if (entry->read_callback) entry->read_callback(addr);
        return bswap_64(*(uint64_t*)(((uint8_t*)entry->mem_block_read) + index));
    }

    return 0;
}

void memory_memcpy(uint32_t dest, uint32_t source, size_t length)
{
    mementry_t* dst_entry;
    mementry_t* src_entry;
    size_t dst_index = 0;
    size_t src_index = 0;

    if (get_final_translation(dest,   &dst_entry, &dst_index, true)  != 0) return;
    if (get_final_translation(source, &src_entry, &src_index, false) != 0) return;

    memcpy(((uint8_t*)dst_entry->mem_block_write) + dst_index, ((uint8_t*)src_entry->mem_block_read) + src_index, length);
}

void* get_real_memory_loc(uint32_t addr)
{
    void* res = NULL;

    uint32_t non_cached_addr = TLB_translate_address(addr);
    mementry_t* entry = get_mementry(non_cached_addr, false);
    if (!entry)
        return NULL;
    non_cached_addr &= 0x1FFFFFFF;
    size_t index = non_cached_addr - entry->base;

    res = (void*)((uint8_t*)entry->mem_block_read + index);

    return res;
}

void* get_framebuffer(void)
{
    return framebuffer_addr;
}