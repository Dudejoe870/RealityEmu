#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "mips/cpu.h"

#define COP0_INDEX       0
#define COP0_RANDOM      1
#define COP0_ENTRYLO0    2
#define COP0_ENTRYLO1    3
#define COP0_CONTEXT     4
#define COP0_PAGEMASK    5
#define COP0_WIRED       6
#define COP0_BADVADDR    8
#define COP0_COUNT       9
#define COP0_ENTRYHI     10
#define COP0_COMPARE     11
#define COP0_STATUS      12
#define COP0_CAUSE       13
#define COP0_EPC         14
#define COP0_PRID        15
#define COP0_CONFIG      16
#define COP0_LLADDR      17
#define COP0_WATCHLO     18
#define COP0_WATCHHI     19
#define COP0_XCONTEXT    20
#define COP0_PARITYERROR 26
#define COP0_CACHEERROR  27
#define COP0_TAGLO       28
#define COP0_TAGHI       29
#define COP0_ERROREPC    30

cpu_t r4300;

double CPU_mhz;

double VIs; // I don't really know where else to store these?
size_t VI_intrs;

void CPU_init(void* ROM, size_t ROM_size);

void CPU_cleanup(void);
