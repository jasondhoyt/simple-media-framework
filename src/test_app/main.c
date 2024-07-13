// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdio.h>

#include <SMF/SMF.h>

static void ErrorCallback(const char *msg, void *data)
{
    fprintf(stderr, "error: %s\n", msg);
}

int main(void)
{
    SMF_SetErrorCallback(ErrorCallback, NULL);
    SMF_Init();

    SMF_SetWindowSize(800, 600);
    SMF_SetWindowScale(2);
    SMF_SetWindowTitle("Test App");
    SMF_CreateWindow();

    SMF_Event evt;
    while (1) {
        if (SMF_PollEvent(&evt) == 1) {
            if (evt.type == SMF_EVENT_TYPE_QUIT) {
                break;
            } else if (evt.type == SMF_EVENT_TYPE_MOUSE_PRESS) {
                printf("mouse press\n");
            } else if (evt.type == SMF_EVENT_TYPE_MOUSE_CLICK) {
                printf("mouse click\n");
            } else if (evt.type == SMF_EVENT_TYPE_MOUSE_DOUBLE_CLICK) {
                printf("mouse double-click\n");
            }
        }

        SMF_Sleep(25);
    }

    SMF_Quit();
    return 0;
}
