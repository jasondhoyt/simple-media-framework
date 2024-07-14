// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <SDL2/SDL.h>

#include "SMF/SMF.h"

#include "SMF_context.h"

#define SMF_WINDOW_TITLE_BUFFER_SIZE 256

static int g_window_width = 800;
static int g_window_height = 600;
static int g_window_scale = 1;
static char g_window_title[SMF_WINDOW_TITLE_BUFFER_SIZE];
static SDL_Window *g_window = NULL;
static SDL_Renderer *g_renderer = NULL;

int SMF_SetWindowSize(int w, int h)
{
    if (SMF_IsInitialized() == -1)
    {
        return -1;
    }

    if (w <= 0)
    {
        return SMF_InvalidArgError("w");
    }

    if (h <= 0)
    {
        return SMF_InvalidArgError("h");
    }

    g_window_width = w;
    g_window_height = h;

    if (!g_window)
    {
        return 0;
    }

    SDL_SetWindowSize(g_window, w * g_window_scale, h * g_window_scale);
    return 0;
}

int SMF_SetWindowScale(int scale)
{
    if (SMF_IsInitialized() == -1)
    {
        return -1;
    }

    if (scale < 1 || scale > 4)
    {
        return SMF_InvalidArgError("scale");
    }

    g_window_scale = scale;

    if (!g_window)
    {
        return 0;
    }

    if (SDL_RenderSetLogicalSize(g_renderer, g_window_width / g_window_scale, g_window_height / g_window_scale) == -1)
    {
        return SMF_SDLError();
    }

    return 0;
}

int SMF_SetWindowTitle(const char *text)
{
    if (SMF_IsInitialized() == -1)
    {
        return -1;
    }

    if (!text)
    {
        return SMF_InvalidArgError("text");
    }

    if (!g_window)
    {
        size_t size = strlen(text);
        if (size > SMF_WINDOW_TITLE_BUFFER_SIZE - 1)
        {
            size = SMF_WINDOW_TITLE_BUFFER_SIZE - 1;
        }

        memcpy(g_window_title, text, size);
        g_window_title[size] = 0;

        return 0;
    }

    SDL_SetWindowTitle(g_window, text);
    return 0;
}

int SMF_CreateWindow(void)
{
    if (g_window)
    {
        return 0;
    }

    g_window = SDL_CreateWindow(g_window_title,
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                g_window_width * g_window_scale,
                                g_window_height * g_window_scale,
                                0);
    if (!g_window)
    {
        return SMF_SDLError();
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!g_renderer)
    {
        SMF_SDLError();
        SDL_DestroyWindow(g_window);
        return -1;
    }

    return 0;
}

void SMF_CleanupWindow(void)
{
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
}

int SMF_IsWindowCreated(void)
{
    if (!g_window)
    {
        return SMF_SetError("window not created");
    }

    return 0;
}

int SMF_GetWindowScale(void)
{
    return g_window_scale;
}
