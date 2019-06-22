#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "r4300/cpu.h"
#include "r4300/mem.h"
#include "config.h"
#include "window.h"
#include "cart.h"

#define WINDOW_WIDTH  960
#define WINDOW_HEIGHT 720

void ReadFileIntoArray(void** buffer, long* len, char* file)
{
    FILE* file_ptr = fopen(file, "rb");
    
    if (!file_ptr)
    {
        printf("Could not open file \"%s\"!\n", file);
        return;
    }

    fseek(file_ptr, 0, SEEK_END);
    *len = ftell(file_ptr);
    rewind(file_ptr);

    *buffer = malloc((*len) * sizeof(char));
    fread(*buffer, *len, 1, file_ptr);
    fclose(file_ptr);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s [ROMFile]\n", argv[0]);
        return 1;
    }

    void* ROM = NULL;
    long len = 0;

    ReadFileIntoArray(&ROM, &len, argv[1]);

    if (!ROM)
    {
        printf("Could not open ROM.\n");
        return 1;
    }

    config.expansion_pak = true;
    config.debug_logging = true;
    config.region       = REG_NTSC;
    config.refresh_rate  = (config.region == REG_NTSC || config.region == REG_MPAL) ? 60 : 50;

    CPU_init(ROM, (size_t)len);

    window_init(WINDOW_WIDTH, WINDOW_HEIGHT, " ");

    while (is_running)
    {
        if (window_run() != 0)
            is_running = false;
    }

    window_cleanup();

    CPU_cleanup();

    return 0;
}