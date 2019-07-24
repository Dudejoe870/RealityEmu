#pragma once

#include "audio.h"
#include "cart.h"
#include "cic.h"
#include "config.h"

#include "mips/cpu.h"
#include "mips/interpreter.h"

#include "r4300/r4300.h"
#include "r4300/debug.h"
#include "r4300/exception.h"
#include "r4300/instutils.h"
#include "r4300/mem.h"
#include "r4300/mi.h"
#include "r4300/opcodetable.h"
#include "r4300/tlb.h"

#include "rdp/bl.h"
#include "rdp/cc.h"
#include "rdp/cmdtable.h"
#include "rdp/interpreter.h"
#include "rdp/rdp.h"
#include "rdp/software_rasterizer.h"

#include "rsp/opcodetable.h"
#include "rsp/rsp.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <getopt.h>
#include <byteswap.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <math.h>