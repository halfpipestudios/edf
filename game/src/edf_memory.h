//
//  edf_memory.h
//  edf
//
//  Created by Manuel Cabrerizo on 19/06/2024.
//

#ifndef EDF_MEMORY_H
#define EDF_MEMORY_H

#include "edf_common.h"
#include <memory.h>

typedef struct Memory {
    void *data;
    sz used;
    sz size;
} Memory;

typedef struct Arena {
    void *data;
    sz used;
    sz size;
} Arena;


Arena arena_create(Memory *memory, sz size);
void *arena_push(Arena *arena, sz size, sz align);
void arena_clear(Arena *arena);

#endif // EDF_MEMORY_H
