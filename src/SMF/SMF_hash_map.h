// Copyright (c) 2024-present, Jason Hoyt
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <stdint.h>

typedef struct SMF_HashMap SMF_HashMap;

SMF_HashMap *SMF_CreateHashMap(void);
void SMF_DestroyHashMap(SMF_HashMap *hash_map);
int SMF_FindHashMapEntry(SMF_HashMap *hash_map, uint64_t key, void **value);
int SMF_InsertHashMapEntry(SMF_HashMap *hash_map, uint64_t key, void *value);
