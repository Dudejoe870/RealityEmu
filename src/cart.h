#pragma once

#include <stdint.h>

typedef struct
{
    uint8_t  PI_BSB_DOM1_LAT_REG;
    uint8_t  PI_BSB_DOM1_PGS_REG;
    uint8_t  PI_BSB_DOM1_PWD_REG;
    uint8_t  PI_BSB_DOM1_RLS_REG;

    uint32_t clock_rate;
    uint32_t boot_addr_offset;
    uint32_t release_offset;
    uint32_t CRC1;
    uint32_t CRC2;
    
    uint64_t unused0;

    char     name[14];

    uint32_t unused1;
    uint16_t Unused2;

    uint8_t  manufacturer_id;
    uint16_t cart_id;
    uint8_t  country_code;

    uint8_t  unused3;
} __attribute__((packed)) cartheader_t;