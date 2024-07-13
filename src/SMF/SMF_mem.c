// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <stdlib.h>

#include "SMF_context.h"

void *SMF_Calloc(size_t count, size_t size)
{
    void *ptr = calloc(count, size);
    if (!ptr) {
        SMF_SetError("out of memory");
        return NULL;
    }

    return ptr;
}

void SMF_Free(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}
