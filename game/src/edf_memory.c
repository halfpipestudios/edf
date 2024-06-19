//
//  edf_memory.c
//  edf
//
//  Created by Manuel Cabrerizo on 19/06/2024.
//

#include "edf_memory.h"

Arena arena_create(Memory *memory, sz size) {
    assert(memory->used + size <= memory->size);
    void *data = memory->data + memory->used;
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
