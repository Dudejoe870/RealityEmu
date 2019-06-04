#include "rdp.h"

#include "../r4300/cpu.h"
#include "../r4300/mem.h"
#include "interpreter.h"
#include "cmdtable.h"

#include <pthread.h>

pthread_cond_t  Cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;

void* RunRDP(void* vargp)
{
    while (IsRunning)
    {
        pthread_mutex_lock(&Lock);
        while (!ShouldRun && IsRunning)
            pthread_cond_wait(&Cond, &Lock);
        pthread_mutex_unlock(&Lock);
        if (!IsRunning) break;
        RDPStep();
    }
    return NULL;
}

void RDPInit(void)
{
    RDP_CMDTableInit();

    pthread_t RDPThread;

    pthread_create(&RDPThread, NULL, RunRDP, NULL);
}

void RDPWakeUp(void)
{
    ShouldRun = true;
    pthread_mutex_lock(&Lock);
    pthread_cond_signal(&Cond);
    pthread_mutex_unlock(&Lock);
}