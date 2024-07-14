// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <SDL2/SDL.h>

#include "SMF/SMF.h"

#include "SMF_context.h"
#include "SMF_window.h"

static int g_is_event_saved = 0;
static SMF_Event g_saved_event;

static int MapMouseButton(int sdl_button, SMF_MouseButton *out)
{
    switch (sdl_button)
    {
    case SDL_BUTTON_LEFT:
        *out = SMF_MOUSE_BUTTON_LEFT;
        return 0;
    case SDL_BUTTON_RIGHT:
        *out = SMF_MOUSE_BUTTON_RIGHT;
        return 0;
    case SDL_BUTTON_MIDDLE:
        *out = SMF_MOUSE_BUTTON_MIDDLE;
        return 0;
    default:
        break;
    }

    return -1;
}

static SMF_KeyMod MapKeyModifiers(int mods)
{
    int out = 0;

    if ((mods & KMOD_SHIFT) != 0)
    {
        out |= SMF_KEY_MOD_SHIFT;
    }

    if ((mods & KMOD_ALT) != 0)
    {
        out |= SMF_KEY_MOD_ALT;
    }

    if ((mods & KMOD_CTRL) != 0)
    {
        out |= SMF_KEY_MOD_CTRL;
    }

    return out;
}

int SMF_PollEvent(SMF_Event *event)
{
    if (SMF_IsInitialized() == -1)
    {
        return -1;
    }

    if (SMF_IsWindowCreated() == -1)
    {
        return -1;
    }

    if (!event)
    {
        return SMF_InvalidArgError("event");
    }

    if (g_is_event_saved)
    {
        *event = g_saved_event;
        g_is_event_saved = 0;
        return 1;
    }

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_WINDOWEVENT:
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                event->type = SMF_EVENT_TYPE_QUIT;
                return 1;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                event->type = SMF_EVENT_TYPE_LOST_FOCUS;
                return 1;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                event->type = SMF_EVENT_TYPE_GOT_FOCUS;
                return 1;
            case SDL_WINDOWEVENT_LEAVE:
                event->type = SMF_EVENT_TYPE_MOUSE_LEAVE;
                return 1;
            case SDL_WINDOWEVENT_ENTER: {
                int scale = SMF_GetWindowScale();
                int x = 0;
                int y = 0;
                SDL_GetMouseState(&x, &y);
                x /= scale;
                y /= scale;
                event->type = SMF_EVENT_TYPE_MOUSE_ENTER;
                event->mouse_enter.x = x;
                event->mouse_enter.y = y;
                return 1;
            }
            default:
                break;
            }
        case SDL_MOUSEMOTION:
            event->type = SMF_EVENT_TYPE_MOUSE_MOVE;
            event->mouse_move.x = e.motion.x;
            event->mouse_move.y = e.motion.y;
            return 1;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if (MapMouseButton(e.button.button, &event->mouse_button.button) == 0)
            {
                event->type = e.type == SDL_MOUSEBUTTONDOWN ? SMF_EVENT_TYPE_MOUSE_PRESS : SMF_EVENT_TYPE_MOUSE_RELEASE;
                event->mouse_move.x = e.button.x;
                event->mouse_move.y = e.button.y;

                if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    g_is_event_saved = 1;
                    g_saved_event.type =
                        e.button.clicks % 2 == 0 ? SMF_EVENT_TYPE_MOUSE_DOUBLE_CLICK : SMF_EVENT_TYPE_MOUSE_CLICK;
                    g_saved_event.mouse_click.button = event->mouse_button.button;
                    g_saved_event.mouse_click.x = e.button.x;
                    g_saved_event.mouse_click.y = e.button.y;
                }

                return 1;
            }
            break;
        case SDL_MOUSEWHEEL:
            event->type = SMF_EVENT_TYPE_MOUSE_SCROLL;
            event->mouse_scroll.x = e.wheel.mouseX;
            event->mouse_scroll.y = e.wheel.mouseY;
            event->mouse_scroll.scroll_x = e.wheel.x;
            event->mouse_scroll.scroll_y = e.wheel.y;
            return 1;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            event->type = e.type == SDL_KEYDOWN ? SMF_EVENT_TYPE_KEY_DOWN : SMF_EVENT_TYPE_KEY_UP;
            event->key.key = (SMF_Key)e.key.keysym.sym;
            event->key.mods = MapKeyModifiers(e.key.keysym.mod);
            return 1;
        case SDL_TEXTINPUT:
            event->type = SMF_EVENT_TYPE_TEXT_INPUT;
            memcpy(event->text_input.text, e.text.text, sizeof(event->text_input.text));
            return 1;
        default:
            break;
        }
    }

    return 0;
}

void SMF_Sleep(int milliseconds)
{
    SDL_Delay(milliseconds);
}

uint64_t SMF_GetTicks(void)
{
    return SDL_GetTicks64();
}
