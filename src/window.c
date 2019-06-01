#include "window.h"

#define GL3_PROTOTYPES 1
#include "GL/glew.h"

#include "SDL2/SDL.h"

#include "mem.h"
#include "cpu.h"

#include <stdint.h>
#include <stdio.h>
#include <byteswap.h>

SDL_Window* window = NULL;
SDL_GLContext context;
GLuint FramebufferTexture = 0;

int WindowInit(int width, int height, char* title)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);

    if (window == NULL)
    {
        printf("Could not initialize the Window!  Error: %s\n", SDL_GetError());
        return 1;
    }

    context = SDL_GL_CreateContext(window);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetSwapInterval(1);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);

    glGenTextures(1, &FramebufferTexture);
    glBindTexture(GL_TEXTURE_2D, FramebufferTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glViewport(0, 0, width, height);

    glClearColor(0, 0, 0, 1);

    return 0;
}

void* GetFramebufferImage(uint32_t* Width, uint32_t* Height, uint8_t* PixelSizeOut)
{
    uint32_t VIStatus = bswap_32(VI_STATUS_REG_RW);
    uint32_t PixelSize = VIStatus & 0b11;
    if (PixelSize == 0) return NULL;

    uint32_t VIhStart = bswap_32(VI_H_START_REG_RW);
    uint32_t HorizontalEndOfVideo = VIhStart & 0x3FF;
    uint32_t HorizontalStartOfVideo = (VIhStart >> 16) & 0x3FF;
    uint32_t XScale = bswap_32(VI_X_SCALE_REG_RW) & 0xFFF;

    uint32_t FramebufferWidth = (HorizontalEndOfVideo - HorizontalStartOfVideo) * (float)XScale / (1 << 10);

    *Width = FramebufferWidth;

    uint32_t VIvStart = bswap_32(VI_V_START_REG_RW);
    uint32_t VerticalEndOfVideo = VIvStart & 0x3FF;
    uint32_t VerticalStartOfVideo = (VIvStart >> 16) & 0x3FF;
    uint32_t YScale = bswap_32(VI_Y_SCALE_REG_RW) & 0xFFF;

    uint32_t FramebufferHeight = ((VerticalEndOfVideo - VerticalStartOfVideo) >> 1) * (float)YScale / (1 << 10);

    *Height = FramebufferHeight;

    void* Framebuffer = GetFramebuffer();

    uint32_t BytesPerPixel = (PixelSize == 3) ? 4 : 2;

    uint8_t* FrameCopy = malloc((FramebufferWidth * FramebufferHeight) * BytesPerPixel);

    uint32_t Pitch = FramebufferWidth * BytesPerPixel;

    uint8_t Interlaced = ((VIStatus & 0b1000000) >> 6) & 1;

    for (size_t y = VerticalStartOfVideo; y < VerticalEndOfVideo; ++y)
    {
        uint32_t Line = (y - VerticalStartOfVideo) >> (~Interlaced & 1);
        uint32_t Offset = Pitch * Line;

        if (BytesPerPixel == 4)
            memcpy(FrameCopy + Offset, ((uint8_t*)Framebuffer) + Offset, Pitch);
        else
        {
            for (size_t End = Offset + Pitch; Offset < End; Offset += 2)
            {
                FrameCopy[Offset]     = ((uint8_t*)Framebuffer)[Offset + 1];
                FrameCopy[Offset + 1] = ((uint8_t*)Framebuffer)[Offset];
            }
        }
    }

    *PixelSizeOut = PixelSize;

    return FrameCopy;
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

    glClear(GL_COLOR_BUFFER_BIT);

    uint8_t PixelSize = 0;
    uint32_t FramebufferWidth  = 0;
    uint32_t FramebufferHeight = 0;
    void* Frame = NULL;
    Frame = GetFramebufferImage(&FramebufferWidth, &FramebufferHeight, &PixelSize);

    if (Frame != NULL)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FramebufferWidth, FramebufferHeight, 0, GL_RGBA, 
                    (PixelSize == 3) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_5_5_1, 
                    Frame);

        free(Frame);

        glBegin(GL_QUADS);

        glTexCoord2f(0, 0); glVertex2f(-1, 1);
        glTexCoord2f(1, 0); glVertex2f(1, 1);
        glTexCoord2f(1, 1); glVertex2f(1, -1);
        glTexCoord2f(0, 1); glVertex2f(-1, -1);

        glEnd();
    }

    SDL_GL_SwapWindow(window);

    return 0;
}

void WindowDeInit(void)
{
    glDeleteTextures(1, &FramebufferTexture);
    SDL_DestroyWindow(window);

    SDL_Quit();
}