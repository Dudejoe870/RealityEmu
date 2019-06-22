#include "rdp.h"

#include "../r4300/cpu.h"
#include "../r4300/mem.h"
#include "interpreter.h"
#include "cmdtable.h"

#include <pthread.h>
#include <sched.h>
#include <stdio.h>

void* RDP_run(void* vargp)
{
    while (is_running && should_run)
        RDP_step();
    return NULL;
}

void RDP_init(void)
{
    RDP_CMDtable_init();
}

void RDP_wake_up(void)
{
    should_run = true;
    pthread_t RDP_thread;

    pthread_create(&RDP_thread, NULL, RDP_run, NULL);
}