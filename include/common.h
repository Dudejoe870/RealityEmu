#pragma once

#include "cart.h"
#include "cic.h"
#include "config.h"
#include "window.h"

#include "r4300/cpu.h"
#include "r4300/exception.h"
#include "r4300/interpreter.h"
#include "r4300/mem.h"
#include "r4300/mi.h"
#include "r4300/opcodetable.h"
#include "r4300/tlb.h"

#include "rdp/cmdtable.h"
#include "rdp/interpreter.h"
#include "rdp/rdp.h"
#include "rdp/software_rasterizer.h"

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