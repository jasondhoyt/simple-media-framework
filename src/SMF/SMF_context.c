// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "SMF/SMF.h"

#include "SMF_context.h"

#include "SMF_window.h"

#define SMF_ERROR_BUF_SIZE 256

static int g_initialized = 0;
static char g_error_buf[SMF_ERROR_BUF_SIZE];
static SMF_ErrorCallback g_error_cb = NULL;
static void *g_error_cb_data = NULL;

int SMF_Init(void)
{
    if (g_initialized == 1) {
        return 0;
    }

    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        return SMF_SDLError();
    }

    g_initialized = 1;

    return 0;
}

void SMF_Quit(void)
{
    if (g_initialized == 0) {
        return;
    }

    SMF_CleanupWindow();

    SDL_Quit();

    g_initialized = 0;
}

const char *SMF_GetError(void)
{
    return g_error_buf;
}

void SMF_SetErrorCallback(SMF_ErrorCallback cb, void *data)
{
    g_error_cb = cb;
    g_error_cb_data = data;
}

int SMF_SetError(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(g_error_buf, sizeof(g_error_buf), format, ap);
    va_end(ap);

    if (g_error_cb) {
        g_error_cb(g_error_buf, g_error_cb_data);
    }

    return -1;
}

int SMF_InvalidArgError(const char *arg)
{
    return SMF_SetError("invalid arg: %s", arg);
}

int SMF_SDLError(void)
{
    const char *buf = SDL_GetError();
    size_t size = strlen(buf);

    if (size > SMF_ERROR_BUF_SIZE - 1) {
        size = SMF_ERROR_BUF_SIZE - 1;
    }

    memcpy(g_error_buf, buf, size);
    return -1;
}

int SMF_IsInitialized(void)
{
    if (g_initialized == 0) {
        SMF_SetError("library not initialized");
        return -1;
    }

    return 0;
}
