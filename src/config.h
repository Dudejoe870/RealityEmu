#pragma once

#include <stdbool.h>

typedef enum
{
    REG_NTSC = 1,
    REG_PAL  = 0,
    REG_MPAL = 2
} region_t;

typedef struct
{
    bool ExpansionPak;
    bool DebugLogging;
    region_t Region;
} config_t;

config_t Config;