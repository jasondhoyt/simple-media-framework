// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

int SMF_InitImages(void);
void SMF_CleanImages(void);
SDL_Surface *SMF_GetImageSurface(uint64_t handle);
SMF_Handle SMF_CreateImageFromSurface(SDL_Surface *surface);
