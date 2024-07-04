//
//  edf_memory.c
//  edf
//
//  Created by Manuel Cabrerizo on 19/06/2024.
//

#include "edf_memory.h"

static Arena scratch_arenas[MAX_SCRATCH_ARENA_COUNT];
static u32 scratch_arenas_count = 0;

Arena arena_create(Memory *memory, sz size) {
    assert(memory->used + size <= memory->size);
    void *data = (u8 *)memory->data + memory->used;
    memory->used = memory->used + size;    
    Arena arena;
    arena.data = data;
    arena.used = 0;
    arena.size = size;
    return arena;
}

void *arena_push(Arena *arena, sz size, sz align) {
    assert(is_power_of_two(align));
    uintptr_t unaligned_addr = (uintptr_t)arena->data + arena->used;
    uintptr_t aligned_addr   = (unaligned_addr + (align - 1)) & ~(align - 1);
    assert(aligned_addr % align == 0);
    sz current_used = (aligned_addr - (uintptr_t)arena->data);
    sz total_used   = current_used + size;
    assert(total_used <= arena->size);
    arena->used  = total_used;
    void *result = (void *)aligned_addr;
    memset(result, 0, size);
    return result;
}

void arena_clear(Arena *arena) {
    arena->used = 0;
}

TempArena temp_arena_begin(Arena *arena) {
    TempArena tmp = {0};
    tmp.arena = arena;
    tmp.pos = arena->used;
    return tmp;
}

void temp_arena_end(TempArena tmp) {
    tmp.arena->used = tmp.pos;
}

void init_scratch_arenas(Memory *memory, u32 count, u64 size) {
    assert(count <= MAX_SCRATCH_ARENA_COUNT);
    scratch_arenas_count = count;
    for(i32 i = 0; i < count; i++) {
        scratch_arenas[i] = arena_create(memory, size);
    }
}

Arena *get_scratch_arena(i32 index) {
    assert(index < scratch_arenas_count);
    return scratch_arenas + index;
}
