#pragma once

#include <stdint.h>
#include <stdbool.h>

void invoke_TLB_miss(uint32_t addr, bool store);
void invoke_break  (void);
void invoke_trap   (void);

void poll_int(void);