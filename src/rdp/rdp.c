#include "rdp/rdp.h"

#include "common.h"

uint8_t RDP_TMEM[4096];
tile_t tiles[8];

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
    if (!should_run)
    {
        should_run = true;
        pthread_t RDP_thread;

        pthread_create(&RDP_thread, NULL, RDP_run, NULL);
    }
}