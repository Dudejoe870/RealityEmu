#pragma once

#include <stdint.h>

typedef struct
{
    uint8_t  PI_BSB_DOM1_LAT_REG;
    uint8_t  PI_BSB_DOM1_PGS_REG;
    uint8_t  PI_BSB_DOM1_PWD_REG;
    uint8_t  PI_BSB_DOM1_RLS_REG;

    uint32_t ClockRate;
    uint32_t BootAddrOffset;
    uint32_t ReleaseOffset;
    uint32_t CRC1;
    uint32_t CRC2;
    
    uint64_t Unused0;

    char     Name[14];

    uint32_t Unused1;
    uint16_t Unused2;

    uint8_t  ManufacturerID;
    uint16_t CartID;
    uint8_t  CountryCode;

    uint8_t  Unused3;
} __attribute__((packed)) cartheader_t;