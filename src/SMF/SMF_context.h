// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

int SMF_SetError(const char *format, ...);

int SMF_InvalidArgError(const char *arg);
int SMF_SDLError(void);

int SMF_IsInitialized(void);
