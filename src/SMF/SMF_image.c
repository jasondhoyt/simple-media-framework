// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "SMF/SMF.h"

#include "SMF_image.h"

#include "SMF_context.h"
#include "SMF_handle_set.h"

typedef struct SMF_Image
{
    SMF_HandleObject base;
    SDL_Surface *surface;
} SMF_Image;

static SMF_HandleSet g_images;

static void DestroyImage(void *data)
{
    SMF_Image *img = (SMF_Image *)data;
    SDL_FreeSurface(img->surface);
}

int SMF_InitImages(void)
{
    return SMF_InitHandleSet(&g_images, SMF_HANDLE_TYPE_IMAGE, sizeof(SMF_Image), DestroyImage);
}

void SMF_CleanImages(void)
{
    SMF_CleanHandleSet(&g_images);
}

SMF_Handle SMF_LoadImage(const char *path)
{
    if (SMF_IsInitialized() == -1) {
        return SMF_INVALID_HANDLE;
    }

    if (!path) {
        SMF_InvalidArgError("path");
        return SMF_INVALID_HANDLE;
    }

    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        SMF_SDLError();
        return SMF_INVALID_HANDLE;
    }

    SMF_Image *image = SMF_CreateHandle(&g_images);
    if (!image) {
        SDL_FreeSurface(surface);
        return SMF_INVALID_HANDLE;
    }

    image->surface = surface;
    return image->base.handle;
}

static int ValidateImageDef(const SMF_ImageDef *def, SDL_Surface *surface)
{
    if (def->x < 0 || def->y < 0) {
        return -1;
    }

    if (def->x >= surface->w || def->y >= surface->h) {
        return -1;
    }

    if (def->w <= 0 || def->h <= 0) {
        return -1;
    }

    if (def->x + def->w > surface->w || def->y + def->h > surface->h) {
        return -1;
    }

    return 0;
}

int SMF_LoadImageSet(const char *path, int count, const SMF_ImageDef *defs, SMF_Handle *handles)
{
    if (SMF_IsInitialized() == -1) {
        return -1;
    }

    if (!path) {
        return SMF_InvalidArgError("path");
    }

    if (count <= 0) {
        return SMF_InvalidArgError("count");
    }

    if (!defs) {
        return SMF_InvalidArgError("defs");
    }

    if (!handles) {
        return SMF_InvalidArgError("handles");
    }

    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        return SMF_SDLError();
    }

    memset(handles, 0, sizeof(SMF_Handle) * count);

    for (int ix = 0; ix < count; ++ix) {
        if (ValidateImageDef(defs + ix, surface) == -1) {
            SDL_FreeSurface(surface);
            return SMF_InvalidArgError("defs");
        }

        SDL_Surface *new_surface = SDL_CreateRGBSurfaceWithFormat(
            0, defs[ix].w, defs[ix].h, surface->format->BitsPerPixel, surface->format->format);
        if (!new_surface) {
            SDL_FreeSurface(surface);
            return SMF_SDLError();
        }

        SDL_Rect src = {defs[ix].x, defs[ix].y, defs[ix].w, defs[ix].h};
        SDL_BlitSurface(surface, &src, new_surface, NULL);

        SMF_Image *img = SMF_CreateHandle(&g_images);
        if (!img) {
            SDL_FreeSurface(new_surface);
            SDL_FreeSurface(surface);
            return -1;
        }

        img->surface = new_surface;
        handles[ix] = img->base.handle;
    }

    SDL_FreeSurface(surface);
    return 0;
}

int SMF_GetImageSize(SMF_Handle image, int *w, int *h)
{
    if (SMF_IsInitialized() == -1) {
        return -1;
    }

    SMF_Image *img = SMF_FindHandleObject(&g_images, image);
    if (!img) {
        return -1;
    }

    if (w) {
        *w = img->surface->w;
    }

    if (h) {
        *h = img->surface->h;
    }

    return 0;
}

SDL_Surface *SMF_GetImageSurface(uint64_t handle)
{
    SMF_Image *image = SMF_FindHandleObject(&g_images, handle);
    if (!image) {
        return NULL;
    }

    return image->surface;
}
