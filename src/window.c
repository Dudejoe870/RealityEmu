#include "window.h"

#define GL3_PROTOTYPES 1
#include "GL/glew.h"

#include "SDL2/SDL.h"

#include "common.h"

SDL_Window* window = NULL;
SDL_GLContext context;
GLuint framebuffer_texture = 0;
cartheader_t* header;

int window_init(int width, int height, char* title)
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

    glGenTextures(1, &framebuffer_texture);
    glBindTexture(GL_TEXTURE_2D, framebuffer_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glViewport(0, 0, width, height);

    glClearColor(0, 0, 0, 1);

    header = get_real_memory_loc(0xB0000000);

    return 0;
}

void* get_framebuffer_image(uint32_t* width, uint32_t* height, uint8_t* pixel_size_out)
{
    uint32_t vi_status = bswap_32(VI_STATUS_REG_RW);
    uint32_t pixel_size = vi_status & 0b11;
    if (pixel_size == 0) return NULL;

    uint32_t vi_h_start = bswap_32(VI_H_START_REG_RW);
    uint32_t h_end_of_video = vi_h_start & 0x3FF;
    uint32_t h_start_of_video = (vi_h_start >> 16) & 0x3FF;
    uint32_t x_scale = bswap_32(VI_X_SCALE_REG_RW) & 0xFFF;

    uint32_t framebuffer_width = (h_end_of_video - h_start_of_video) * (float)x_scale / (1 << 10);

    *width = framebuffer_width;

    uint32_t vi_v_start = bswap_32(VI_V_START_REG_RW);
    uint32_t v_end_of_video = vi_v_start & 0x3FF;
    uint32_t v_start_of_video = (vi_v_start >> 16) & 0x3FF;
    uint32_t y_scale = bswap_32(VI_Y_SCALE_REG_RW) & 0xFFF;

    uint32_t framebuffer_height = ((v_end_of_video - v_start_of_video) >> 1) * (float)y_scale / (1 << 10);

    *height = framebuffer_height;

    void* framebuffer = get_framebuffer();

    uint32_t bytes_per_pixel = (pixel_size == 3) ? 4 : 2;

    uint8_t* frame_copy = malloc((framebuffer_width * framebuffer_height) * bytes_per_pixel);

    uint32_t pitch = framebuffer_width * bytes_per_pixel;

    uint8_t interlaced = ((vi_status & 0b1000000) >> 6) & 1;

    for (size_t y = v_start_of_video; y < v_end_of_video; ++y)
    {
        uint32_t line = (y - v_start_of_video) >> (~interlaced & 1);
        uint32_t offset = pitch * line;

        if (bytes_per_pixel == 4)
            memcpy(frame_copy + offset, ((uint8_t*)framebuffer) + offset, pitch);
        else
        {
            for (size_t end = offset + pitch; offset < end; offset += 2)
            {
                frame_copy[offset]     = ((uint8_t*)framebuffer)[offset + 1];
                frame_copy[offset + 1] = ((uint8_t*)framebuffer)[offset];
            }
        }
    }

    *pixel_size_out = pixel_size;

    return frame_copy;
}

int window_run(void)
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

    uint8_t  pixel_size = 0;
    uint32_t framebuffer_width  = 0;
    uint32_t framebuffer_height = 0;
    void*    frame = get_framebuffer_image(&framebuffer_width, &framebuffer_height, &pixel_size);

    if (frame != NULL)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebuffer_width, framebuffer_height, 0, GL_RGBA, 
                    (pixel_size == 3) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_5_5_1, 
                    frame);

        free(frame);

        glBegin(GL_QUADS);

        glTexCoord2f(0, 0); glVertex2f(-1, 1);
        glTexCoord2f(1, 0); glVertex2f(1, 1);
        glTexCoord2f(1, 1); glVertex2f(1, -1);
        glTexCoord2f(0, 1); glVertex2f(-1, -1);

        glEnd();
    }

    char win_name[64];
    strcpy(win_name, "RealityEmu - ");
    strcat(win_name, header->name);
    strcat(win_name, " CPU: ");
    char mhz_str[64];
    sprintf(mhz_str, "%.2f", (float)CPU_mhz);
    strcat(win_name, mhz_str);
    strcat(win_name, " MHz");
    SDL_SetWindowTitle(window, win_name);

    SDL_GL_SwapWindow(window);

    return 0;
}

void window_cleanup(void)
{
    glDeleteTextures(1, &framebuffer_texture);
    SDL_DestroyWindow(window);

    SDL_Quit();
}