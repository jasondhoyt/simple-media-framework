// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <assert.h>

#include "SMF_hash_map.h"

#include "SMF_mem.h"

#define USE_UNUSED 0
#define USE_ACTIVE 1
#define USE_ERASED 2

#define INITIAL_SLOT_CAPACITY 16
#define PERTURB_SHIFT 5
#define MAX_LOAD_FACTOR 0.75

typedef struct SMF_HashMapSlot
{
    int use;
    uint64_t key;
    void *value;
} SMF_HashMapSlot;

typedef struct SMF_HashMap
{
    uint64_t cap;
    uint64_t size;
    SMF_HashMapSlot *slots;
} SMF_HashMap;

SMF_HashMap *SMF_CreateHashMap(void)
{
    SMF_HashMap *hash_map = SMF_Calloc(1, sizeof(SMF_HashMap));
    if (!hash_map)
    {
        return NULL;
    }

    hash_map->cap = INITIAL_SLOT_CAPACITY;
    hash_map->slots = SMF_Calloc(hash_map->cap, sizeof(SMF_HashMapSlot));
    if (!hash_map->slots)
    {
        SMF_Free(hash_map);
        return NULL;
    }

    return hash_map;
}

void SMF_DestroyHashMap(SMF_HashMap *hash_map)
{
    if (hash_map)
    {
        SMF_Free(hash_map->slots);
        SMF_Free(hash_map);
    }
}

int SMF_FindHashMapEntry(SMF_HashMap *hash_map, uint64_t key, void **value)
{
    assert(hash_map);
    assert(value);

    uint64_t mask = hash_map->cap - 1;
    uint64_t perturb = key;
    uint64_t ix = key & mask;
    while (hash_map->slots[ix].use != USE_UNUSED)
    {
        if (hash_map->slots[ix].use == USE_ACTIVE && hash_map->slots[ix].key == key)
        {
            *value = hash_map->slots[ix].value;
            return 1;
        }

        perturb >>= PERTURB_SHIFT;
        ix = (5 * ix) + 1 + perturb;
    }

    return 0;
}

static void InsertIntoHashMap(SMF_HashMap *hash_map, uint64_t key, void *value)
{
    uint64_t mask = hash_map->cap - 1;
    uint64_t perturb = key;
    uint64_t ix = key & mask;
    while (hash_map->slots[ix].use == USE_ACTIVE)
    {
        perturb >>= PERTURB_SHIFT;
        ix = (5 * ix) + 1 + perturb;
    }

    hash_map->slots[ix].use = USE_ACTIVE;
    hash_map->slots[ix].key = key;
    hash_map->slots[ix].value = value;
    hash_map->size++;
}

static int ExpandHashMap(SMF_HashMap *hash_map)
{
    uint64_t new_cap = hash_map->cap * 2;
    SMF_HashMapSlot *new_slots = SMF_Calloc(new_cap, sizeof(SMF_HashMapSlot));
    if (!new_slots)
    {
        return -1;
    }

    uint64_t old_cap = hash_map->cap;
    SMF_HashMapSlot *old_slots = hash_map->slots;

    hash_map->cap = new_cap;
    hash_map->slots = new_slots;
    hash_map->size = 0;

    for (uint64_t ix = 0; ix < old_cap; ++ix)
    {
        if (old_slots[ix].use == USE_ACTIVE)
        {
            InsertIntoHashMap(hash_map, old_slots[ix].key, old_slots[ix].value);
        }
    }

    SMF_Free(old_slots);
    return 0;
}

int SMF_InsertHashMapEntry(SMF_HashMap *hash_map, uint64_t key, void *value)
{
    assert(hash_map);

    double load_factor = (double)hash_map->size / (double)hash_map->cap;
    if (load_factor >= MAX_LOAD_FACTOR)
    {
        if (ExpandHashMap(hash_map) == -1)
        {
            return -1;
        }
    }

    InsertIntoHashMap(hash_map, key, value);
    return 0;
}
