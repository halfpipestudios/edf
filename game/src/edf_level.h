//
//  edf_level.h
//  edf
//
//  Created by Manuel Cabrerizo on 28/06/2024.
//

#ifndef EDF_LEVEL_H
#define EDF_LEVEL_H

#include "edf_graphics.h"

typedef struct Level {
    struct GameState *gs;

    struct Arena *arena;
    struct EntityManager *em;

    R2f dim;
    V3 camera_vel;
    V3 camera_pos;

} Level;

struct GameState;

Level *load_level_from_file(struct GameState *gs, char *path, struct Arena *arena, struct EntityManager *em);
Level *load_level(struct GameState *gs, struct Arena *arena, struct EntityManager *em);
void level_update(Level *level, f32 dt);
void level_render(Level *level, Gpu gpu);

#endif /* EDF_LEVEL_H */
