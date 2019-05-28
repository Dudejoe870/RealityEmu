#include "window.h"

#include "SDL2/SDL.h"
#include "mem.h"

#include <stdint.h>
#include <stdio.h>
#include <byteswap.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* screen8888 = NULL;
SDL_Texture* screen5551 = NULL;

int WindowInit(int width, int height, char* title)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);

    if (window == NULL)
    {
        printf("Could not initialize the Window!  Error: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL)
    {
        printf("Could not create the Renderer!  Error: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    screen8888 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 640, 480);

    if (screen8888 == NULL)
    {
        printf("Could not create screen texture!  \"%s\"\n", SDL_GetError());
        return 1;
    }

    screen5551 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, 640, 480);

    if (screen5551 == NULL)
    {
        printf("Could not create screen texture!  \"%s\"\n", SDL_GetError());
        return 1;
    }

    return 0;
}

bool RenderFormat = false; // False: RGBA8888, True: RGBA5551
bool ShouldRender = false;
uint32_t FrameWidth = 0;
uint32_t FrameHeight = 0;

void RenderFramebuffer(void)
{
    uint32_t VIStatus = bswap_32(VI_STATUS_REG_RW);
    uint32_t PixelSize = VIStatus & 0b11;

    ShouldRender = PixelSize > 0;

    if (!ShouldRender) return;

    uint32_t FramebufferWidth = bswap_32(VI_WIDTH_REG_RW);

    FrameWidth = FramebufferWidth;

    uint8_t Interlace = (uint8_t)((VIStatus & 0b1000000) >> 6);
    uint32_t VIvStart = bswap_32(VI_V_START_REG_RW);
    uint32_t VerticalEndOfVideo = VIvStart & 0x3FF;
    uint32_t VerticalStartOfVideo = (VIvStart >> 16) & 0x3FF;

    uint32_t FramebufferHeight = ((VerticalEndOfVideo - VerticalStartOfVideo) + 6) >> (~Interlace & 0x01);
    
    FrameHeight = FramebufferHeight;

    void* Framebuffer = GetFramebuffer();

    if (FramebufferWidth > 640 || FramebufferHeight > 480) return;

    SDL_Rect LockedArea;
    LockedArea.x = 0;
    LockedArea.y = 0;
    LockedArea.w = FramebufferWidth;
    LockedArea.h = FramebufferHeight;

    void* Pixels;
    int Pitch;
    if (PixelSize == 2) // RGBA5551
    {
        SDL_LockTexture(screen5551, &LockedArea, &Pixels, &Pitch);

        memcpy(Pixels, Framebuffer, (FramebufferWidth * FramebufferHeight) * 2);

        SDL_UnlockTexture(screen5551);
        RenderFormat = true;
    }
    else if (PixelSize == 3) // RGBA8888
    {
        SDL_LockTexture(screen8888, &LockedArea, &Pixels, &Pitch);

        memcpy(Pixels, Framebuffer, (FramebufferWidth * FramebufferHeight) * 4);

        SDL_UnlockTexture(screen8888);
        RenderFormat = false;
    }
}

int WindowRun(void)
{
    SDL_Event e;
    if (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
            case SDL_QUIT:
                return 1;
        }
    }

    RenderFramebuffer();

    if (ShouldRender)
    {
        SDL_Rect TexRect;
        TexRect.x = 0;
        TexRect.y = 0;
        TexRect.w = FrameWidth;
        TexRect.h = FrameHeight;

        SDL_RenderCopy(renderer, RenderFormat ? screen5551 : screen8888, &TexRect, NULL);
    }

    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
    return 0;
}

void WindowDeInit(void)
{
    SDL_DestroyTexture(screen8888);
    SDL_DestroyTexture(screen5551);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}