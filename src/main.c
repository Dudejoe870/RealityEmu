#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "config.h"
#include "window.h"
#include "cart.h"

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

    Config.ExpansionPak = true;
    Config.DebugLogging = true;

    CPUInit(ROM, (size_t)len);
    
    cartheader_t* Header = ROM;

    char WinName[32];
    strcpy(WinName, "RealityEmu - ");
    strcat(WinName, Header->Name);

    WindowInit(960, 720, WinName);

    while (IsRunning)
    {
        if (WindowRun() != 0)
            IsRunning = false;
    }

    WindowDeInit();

    CPUDeInit();

    return 0;
}