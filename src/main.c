#include "common.h"

#include "SDL2/SDL.h"

#define WINDOW_WIDTH  960
#define WINDOW_HEIGHT 720

void read_file_into_array(void** buffer, long* len, char* file)
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
    int opt;

    bool stop_running = false;

    struct option long_options[] =
    {
        { "cart",             required_argument, 0, 'c' },
        { "region",           required_argument, 0, 'r' },
        { "no-expansion-pak", no_argument,       0,  0  },
        { "debug",            no_argument,       0,  0  },
        { "debug-cc",         no_argument,       0,  0  },
        { "help",             no_argument,       0, 'h' },
        { "nearest-neighbor", no_argument,       0,  0  },
        { 0,      0,                             0,  0  }
    };

    int option_index;

    void* ROM = NULL;
    long  len = 0;

    config.region        = REG_NTSC;
    config.expansion_pak = true;
    config.debug_logging = false;
    config.gfx_type      = GFX_LINEAR;
    config.cc_logging    = false;

    while ((opt = getopt_long(argc, argv, ":c:r:h", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
            case 0:
                if (strcmp(long_options[option_index].name, "no-expansion-pak") == 0)
                    config.expansion_pak = false;
                else if (strcmp(long_options[option_index].name, "debug") == 0)
                    config.debug_logging = true;
                else if (strcmp(long_options[option_index].name, "debug-cc") == 0)
                    config.cc_logging = true;
                else if (strcmp(long_options[option_index].name, "nearest-neighbor") == 0)
                    config.gfx_type = GFX_NEAREST;
                else
                {
                    puts("Unknown Option");
                    stop_running = true;
                }
                break;
            case 'c':
                read_file_into_array(&ROM, &len, optarg);
                break;
            case 'r':
                if (strcmp(optarg, "NTSC") == 0)
                    config.region = REG_NTSC;
                else if (strcmp(optarg, "PAL") == 0)
                    config.region = REG_PAL;
                else if (strcmp(optarg, "MPAL") == 0)
                    config.region = REG_MPAL;
                else
                {
                    printf("%s isn't a valid Region.\n", optarg);
                    stop_running = true;
                }
                break;
            case 'h':
                stop_running = true;
                break;
            case ':':
                printf("%s needs value!\n", long_options[option_index].name);
                stop_running = true;
                break;
            case '?':
                puts("Unknown Option");
                stop_running = true;
                break;
        }
    }

    if (stop_running)
    {
        printf("Usage: %s [Options]\n Options:\n  --cart (-c) [ROM]: Sets the ROM.\n  --region (-r) [Region (NTSC, PAL, MPAL)]: Sets the Region of the emulated Console.\n", argv[0]);
        puts("  --no-expansion-pak: Disables the Expansion Pak.\n  --debug: Enables Debug Logging.\n  --help (-h): Displays this help message.");
        puts("  --nearest-neighbor: Turns on nearest neighbor interpolation on the framebuffer.");
        return 1;
    }

    if (!ROM)
    {
        puts("Could not open ROM.");
        return 1;
    }

    config.refresh_rate = (config.region == REG_NTSC || config.region == REG_MPAL) ? 60 : 50;

    switch (config.region)
    {
        case REG_NTSC: config.vi_clock = 48681812; break;
        case REG_PAL:  config.vi_clock = 49656530; break;
        case REG_MPAL: config.vi_clock = 48628316; break;
    }

    CPU_init(ROM, (size_t)len);

    window_init(WINDOW_WIDTH, WINDOW_HEIGHT, " ");

    float time_seconds;

    while (is_running)
    {
        if (window_run() != 0)
            is_running = false;
        time_seconds = ((float)SDL_GetTicks()) / 1000;

        CPU_mhz = (r4300.all_cycles / 1000000) / time_seconds;
        RSP_mhz = (rsp.all_cycles   / 1000000) / time_seconds;
        VIs     = VI_intrs / time_seconds;
    }

    window_cleanup();

    CPU_cleanup();

    return 0;
}