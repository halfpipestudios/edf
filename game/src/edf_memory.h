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

typedef struct TempArena {
    Arena *arena;
    u64 pos;
} TempArena;

Arena arena_create(Memory *memory, sz size);
void *arena_push(Arena *arena, sz size, sz align);
void arena_clear(Arena *arena);

TempArena temp_arena_begin(Arena *arena);
void temp_arena_end(TempArena tmp);

#define MAX_SCRATCH_ARENA_COUNT 3
void init_scratch_arenas(Memory *memory, u32 count, u64 size);
Arena *get_scratch_arena(i32 index);

#endif // EDF_MEMORY_H
