#include "cic.h"

#include "r4300/mem.h"

#include <stdio.h>

uint32_t CRC32(uint32_t Base, size_t size) 
{
    uint32_t table[256];
    unsigned n, k;
    uint32_t c;

    for (n = 0; n < 256; ++n) 
    {
        c = (uint32_t)n;

        for (k = 0; k < 8; ++k) 
        {
            if (c & 1)
                c = 0xEDB88320L ^ (c >> 1);
            else
                c = c >> 1;
        }  

        table[n] = c;
    }

    c = 0L ^ 0xFFFFFFFF;

    for (n = 0; n < size; ++n)
        c = table[(c ^ ReadUInt8(Base + n)) & 0xFF] ^ (c >> 8);

    return c ^ 0xFFFFFFFF;
}

// All values from Cen64: https://github.com/tj90241/cen64/blob/master/si/cic.c
#define CIC_SEED_NUS_5101 0x0000AC00U
#define CIC_SEED_NUS_6101 0x00043F3FU
#define CIC_SEED_NUS_6102 0x00003F3FU
#define CIC_SEED_NUS_6103 0x0000783FU
#define CIC_SEED_NUS_6105 0x0000913FU
#define CIC_SEED_NUS_6106 0x0000853FU
#define CIC_SEED_NUS_8303 0x0000DD00U

#define CRC_NUS_5101 0x587BD543U
#define CRC_NUS_6101 0x6170A4A1U
#define CRC_NUS_7102 0x009E9EA3U
#define CRC_NUS_6102 0x90BB6CB5U
#define CRC_NUS_6103 0x0B050EE0U
#define CRC_NUS_6105 0x98BC2C86U
#define CRC_NUS_6106 0xACC8580AU
#define CRC_NUS_8303 0x0E018159U
#define CRC_iQue_1 0xCD19FEF1U
#define CRC_iQue_2 0xB98CED9AU
#define CRC_iQue_3 0xE71C2766U

uint32_t GetCICSeed(void)
{
    uint32_t CRC        = CRC32(0x10000040, 0xFC0);
    uint32_t Aleck64CRC = CRC32(0x10000040, 0xBC0);

    if (Aleck64CRC == CRC_NUS_5101) return CIC_SEED_NUS_5101;
    switch (CRC)
    {
        default:
            fprintf(stderr, "WARNING: Unknown CIC CRC \"0x%x\", defaulting to seed CIC-6101\n", CRC);
            return CIC_SEED_NUS_6101;
        
        case CRC_NUS_6101:
        case CRC_NUS_7102:
        case CRC_iQue_1:
        case CRC_iQue_2:
        case CRC_iQue_3:
            return CIC_SEED_NUS_6101;

        case CRC_NUS_6102:
            return CIC_SEED_NUS_6102;

        case CRC_NUS_6103:
            return CIC_SEED_NUS_6103;

        case CRC_NUS_6105:
            return CIC_SEED_NUS_6105;

        case CRC_NUS_6106:
            return CIC_SEED_NUS_6106;

        case CRC_NUS_8303:
            return CIC_SEED_NUS_8303;
    }
}