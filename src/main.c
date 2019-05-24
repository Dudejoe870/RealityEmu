#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "config.h"

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

    void* ROM;
    long len;

    ReadFileIntoArray(&ROM, &len, argv[1]);

    if (!ROM)
    {
        printf("Could not open ROM.\n");
        return 1;
    }

    Config.ExpansionPak = true;

    CPUInit(ROM, (size_t)len);
    
    CPUDeInit();

    return 0;
}