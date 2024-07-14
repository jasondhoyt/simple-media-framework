// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "SMF/SMF.h"

#include "SMF_image.h"

#include "SMF_context.h"
#include "SMF_handle_set.h"
#include "SMF_hash_map.h"

typedef struct SMF_Font
{
    SMF_HandleObject base;
    TTF_Font *ttf;
    SMF_Handle ascii_map[95];
    SMF_HashMap *glyph_map;
    int height;
    int is_fixed_width;
    int x_adjust;
} SMF_Font;

static SMF_HandleSet g_fonts;

static void DestroyFont(void *data)
{
    SMF_Font *font = (SMF_Font *)data;
    if (font->glyph_map)
    {
        SMF_DestroyHashMap(font->glyph_map);
    }
    if (font->ttf)
    {
        TTF_CloseFont(font->ttf);
    }
}

int SMF_InitFonts(void)
{
    return SMF_InitHandleSet(&g_fonts, SMF_HANDLE_TYPE_FONT, sizeof(SMF_Font), DestroyFont);
}

void SMF_CleanFonts(void)
{
    SMF_CleanHandleSet(&g_fonts);
}

int AddGlyphSurfaceToFont(SMF_Font *font, uint32_t glyph, SDL_Surface *glyph_surface)
{
    SMF_Handle glyph_image = SMF_CreateImageFromSurface(glyph_surface);
    if (glyph_image == SMF_INVALID_HANDLE)
    {
        SDL_FreeSurface(glyph_surface);
        return -1;
    }

    if (glyph >= 32 && glyph < 128)
    {
        font->ascii_map[glyph - 32] = glyph_image;
        return 0;
    }

    if (!font->glyph_map)
    {
        font->glyph_map = SMF_CreateHashMap();
        if (!font->glyph_map)
        {
            return -1;
        }
    }

    return SMF_InsertHashMapEntry(font->glyph_map, glyph, (void *)glyph_image);
}

SMF_Handle SMF_LoadTrueTypeFont(const char *path, int ttf_size)
{
    if (SMF_IsInitialized() == -1)
    {
        return SMF_INVALID_HANDLE;
    }

    TTF_Font *ttf = TTF_OpenFont(path, ttf_size);
    if (!ttf)
    {
        SMF_SDLError();
        return SMF_INVALID_HANDLE;
    }

    SMF_Font *font = SMF_CreateHandle(&g_fonts);
    if (!font)
    {
        TTF_CloseFont(ttf);
        return SMF_INVALID_HANDLE;
    }

    font->ttf = ttf;
    font->height = TTF_FontHeight(ttf);
    font->is_fixed_width = TTF_FontFaceIsFixedWidth(ttf) != 0;
    font->x_adjust = 0;

    SDL_Color color = {255, 255, 255};
    for (int ix = 32; ix < 128; ++ix)
    {
        SDL_Surface *glyph_surface = TTF_RenderGlyph32_Blended(ttf, ix, color);
        if (glyph_surface)
        {
            AddGlyphSurfaceToFont(font, ix, glyph_surface);
        }
    }

    return font->base.handle;
}

SMF_Handle SMF_LoadBitmapFont(const char *path, int glyph_count, const SMF_GlyphDef *glyphs, int height, int x_adjust)
{
    if (!path)
    {
        SMF_InvalidArgError("path");
        return SMF_INVALID_HANDLE;
    }

    if (glyph_count <= 0)
    {
        SMF_InvalidArgError("glyph_count");
        return SMF_INVALID_HANDLE;
    }

    if (!glyphs)
    {
        SMF_InvalidArgError("glyphs");
        return SMF_INVALID_HANDLE;
    }

    if (height <= 0)
    {
        SMF_InvalidArgError("height");
        return SMF_INVALID_HANDLE;
    }

    if (SMF_IsInitialized() == -1)
    {
        return SMF_INVALID_HANDLE;
    }

    SDL_Surface *surface = IMG_Load(path);
    if (!surface)
    {
        SMF_SDLError();
        return SMF_INVALID_HANDLE;
    }

    SMF_Font *font = SMF_CreateHandle(&g_fonts);
    if (!font)
    {
        SDL_FreeSurface(surface);
        return SMF_INVALID_HANDLE;
    }

    font->ttf = NULL;
    font->glyph_map = NULL;
    font->height = height;
    font->x_adjust = x_adjust;

    int last_width = 0;
    for (int i = 0; i < glyph_count; ++i)
    {
        const SMF_GlyphDef *def = glyphs + i;
        if (def->x >= 0 && def->x < surface->w && def->y >= 0 && def->y < surface->h && def->w > 0 &&
            def->x + def->w <= surface->w && def->y + height <= surface->h)
        {
            SDL_Surface *glyph_surface = SDL_CreateRGBSurfaceWithFormat(
                0, def->w, height, surface->format->BitsPerPixel, surface->format->format);
            if (glyph_surface)
            {
                SDL_Rect src = {def->x, def->y, def->w, height};
                SDL_BlitSurface(surface, &src, glyph_surface, NULL);

                AddGlyphSurfaceToFont(font, def->glyph, glyph_surface);
            }
        }
    }

    SDL_FreeSurface(surface);
    return font->base.handle;
}

int SMF_GetFontHeight(SMF_Handle font)
{
    if (SMF_IsInitialized() == -1)
    {
        return -1;
    }

    SMF_Font *data = SMF_FindHandleObject(&g_fonts, font);
    if (!data)
    {
        return -1;
    }

    return data->height;
}

int SMF_IsFontFixedWidth(SMF_Handle font)
{
    if (SMF_IsInitialized() == -1)
    {
        return -1;
    }

    SMF_Font *data = SMF_FindHandleObject(&g_fonts, font);
    if (!data)
    {
        return -1;
    }

    return data->is_fixed_width;
}

int SMF_FontContainsGlyph(SMF_Handle font, uint32_t glyph)
{
    if (SMF_IsInitialized() == -1)
    {
        return -1;
    }

    SMF_Font *data = SMF_FindHandleObject(&g_fonts, font);
    if (!data)
    {
        return -1;
    }

    if (glyph >= 32 && glyph < 128)
    {
        return data->ascii_map[glyph - 32] != SMF_INVALID_HANDLE ? 1 : 0;
    }

    if (data->glyph_map)
    {
        SMF_Handle image = SMF_INVALID_HANDLE;
        if (SMF_FindHashMapEntry(data->glyph_map, glyph, (void **)&image) == 1)
        {
            return 1;
        }
    }

    if (data->ttf)
    {
        return TTF_GlyphIsProvided32(data->ttf, glyph) != 0;
    }

    return 0;
}

static SMF_Handle GetFontGlyphImage(SMF_Font *data, uint32_t glyph)
{
    if (glyph >= 32 && glyph < 128)
    {
        return data->ascii_map[glyph - 32] != SMF_INVALID_HANDLE ? 1 : 0;
    }

    if (data->glyph_map)
    {
        SMF_Handle image = SMF_INVALID_HANDLE;
        if (SMF_FindHashMapEntry(data->glyph_map, glyph, (void **)&image) == 1)
        {
            return image;
        }
    }

    if (data->ttf)
    {
        SDL_Color color = {255, 255, 255};
        SDL_Surface *glyph_surface = TTF_RenderGlyph32_Blended(data->ttf, glyph, color);
        if (glyph_surface)
        {
            SMF_Handle glyph_image = SMF_CreateImageFromSurface(glyph_surface);
            if (glyph_image != SMF_INVALID_HANDLE)
            {
                if (!data->glyph_map)
                {
                    data->glyph_map = SMF_CreateHashMap();
                    if (!data->glyph_map)
                    {
                        SDL_FreeSurface(glyph_surface);
                        return SMF_INVALID_HANDLE;
                    }
                }

                if (SMF_InsertHashMapEntry(data->glyph_map, glyph, (void *)glyph_image) == 1)
                {
                    return glyph_image;
                }
            }
            else
            {
                SDL_FreeSurface(glyph_surface);
            }
        }
    }

    return SMF_INVALID_HANDLE;
}

SMF_Handle SMF_GetFontGlyphImage(SMF_Handle font, uint32_t glyph)
{
    if (SMF_IsInitialized() == -1)
    {
        return SMF_INVALID_HANDLE;
    }

    SMF_Font *data = SMF_FindHandleObject(&g_fonts, font);
    if (!data)
    {
        return SMF_INVALID_HANDLE;
    }

    return GetFontGlyphImage(data, glyph);
}

int SMF_CalcTextWidth(SMF_Handle font, const char *text)
{
    if (SMF_IsInitialized() == -1)
    {
        return SMF_INVALID_HANDLE;
    }

    SMF_Font *data = SMF_FindHandleObject(&g_fonts, font);
    if (!data)
    {
        return SMF_INVALID_HANDLE;
    }

    int width = 0;
    for (const char *c = text; *c; ++c)
    {
        SMF_Handle glyph_image = GetFontGlyphImage(data, *c);
        if (glyph_image != SMF_INVALID_HANDLE)
        {
            int w = 0;
            if (SMF_GetImageSize(glyph_image, &w, NULL) == 0)
            {
                width += (w + data->x_adjust);
            }
        }
    }

    return width;
}
