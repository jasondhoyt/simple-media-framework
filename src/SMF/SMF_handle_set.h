// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum SMF_HandleType
{
    SMF_HANDLE_TYPE_IMAGE = 1,
    SMF_HANDLE_TYPE_FONT
} SMF_HandleType;

typedef struct SMF_HandleObject
{
    uint64_t handle;
} SMF_HandleObject;

#define SMF_MAKE_HANDLE(Type, Index, Id) ((Type & 0xff) | ((Index & 0xffffff) << 8) | ((Id & 0xffffffff) << 32))

#define SMF_HANDLE_TYPE(Handle) (Handle & 0xff)
#define SMF_HANDLE_INDEX(Handle) ((Handle >> 8) & 0xffffff)

typedef struct SMF_HandleSet
{
    SMF_HandleType type;
    size_t data_size;
    void (*clean_cb)(void *);
    uint64_t next_id;
    uint64_t data_cap;
    uint64_t data_len;
    char *data;
} SMF_HandleSet;

int SMF_InitHandleSet(SMF_HandleSet *handle_set, SMF_HandleType type, size_t data_size, void (*clean_cb)(void *));
void SMF_CleanHandleSet(SMF_HandleSet *handle_set);

void *SMF_CreateHandle(SMF_HandleSet *handle_set);
void *SMF_FindHandleObject(SMF_HandleSet *handle_set, uint64_t handle);
