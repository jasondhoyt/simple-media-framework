// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <assert.h>
#include <string.h>

#include "SMF_handle_set.h"

#include "SMF_context.h"
#include "SMF_mem.h"

int SMF_InitHandleSet(SMF_HandleSet *handle_set, SMF_HandleType type, size_t data_size, void (*clean_cb)(void *))
{
    assert(handle_set);
    assert(type >= SMF_HANDLE_TYPE_IMAGE);
    assert(type <= SMF_HANDLE_TYPE_FONT);
    assert(data_size >= sizeof(SMF_HandleObject));
    assert(clean_cb);

    handle_set->type = type;
    handle_set->data_size = data_size;
    handle_set->clean_cb = clean_cb;
    handle_set->next_id = 1;
    handle_set->data_cap = 16;
    handle_set->data_len = 0;
    handle_set->data = SMF_Calloc(handle_set->data_cap, handle_set->data_size);
    if (!handle_set->data)
    {
        return -1;
    }

    return 0;
}

void SMF_CleanHandleSet(SMF_HandleSet *handle_set)
{
    assert(handle_set);

    for (uint64_t ix = 0; ix < handle_set->data_len; ++ix)
    {
        SMF_HandleObject *obj = (SMF_HandleObject *)(handle_set->data + (ix * handle_set->data_size));
        if (obj->handle != 0)
        {
            handle_set->clean_cb(obj);
        }
    }

    SMF_Free(handle_set->data);
}

void *SMF_CreateHandle(SMF_HandleSet *handle_set)
{
    assert(handle_set);

    if (handle_set->next_id == 0xffffffff)
    {
        SMF_SetError("too many handles");
        return NULL;
    }

    if (handle_set->data_len == handle_set->data_cap)
    {
        if (handle_set->data_cap >= 0x1000000)
        {
            SMF_SetError("too many objects");
            return NULL;
        }

        uint64_t new_cap = handle_set->data_cap * 2;
        if (handle_set->data_cap > 0x1000000)
        {
            new_cap = 0x1000000;
        }

        char *new_data = SMF_Calloc(new_cap, handle_set->data_size);
        if (!new_data)
        {
            return NULL;
        }

        memcpy(new_data, handle_set->data, handle_set->data_len * handle_set->data_size);
        SMF_Free(handle_set->data);

        handle_set->data = new_data;
        handle_set->data_cap = new_cap;
    }

    SMF_HandleObject *obj = (SMF_HandleObject *)(handle_set->data + (handle_set->data_len * handle_set->data_size));
    uint64_t ix = handle_set->data_len;
    handle_set->data_len++;

    uint64_t id = handle_set->next_id + 1;

    obj->handle = SMF_MAKE_HANDLE(handle_set->type, ix, id);

    return obj;
}

void *SMF_FindHandleObject(SMF_HandleSet *handle_set, uint64_t handle)
{
    assert(handle_set);

    if (handle == 0)
    {
        SMF_SetError("invalid handle");
        return NULL;
    }

    SMF_HandleType type = (SMF_HandleType)SMF_HANDLE_TYPE(handle);
    if (type != handle_set->type)
    {
        SMF_SetError("invalid handle");
        return NULL;
    }

    uint64_t ix = SMF_HANDLE_INDEX(handle);
    if (ix >= handle_set->data_len)
    {
        SMF_SetError("invalid handle");
        return NULL;
    }

    SMF_HandleObject *obj = (SMF_HandleObject *)(handle_set->data + (ix * handle_set->data_size));
    if (obj->handle != handle)
    {
        SMF_SetError("invalid handle");
        return NULL;
    }

    return obj;
}
