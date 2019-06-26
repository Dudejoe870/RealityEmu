#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    REG_NTSC = 1,
    REG_PAL  = 0,
    REG_MPAL = 2
} region_t;

typedef enum
{
    ROM_GAMEPACK = 0,
    ROM_DD       = 1
} romtype_t;

typedef struct
{
    bool     expansion_pak;
    bool     debug_logging;
    region_t region;
    uint8_t  refresh_rate;
} config_t;

config_t config;