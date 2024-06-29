//
//  edf_level.c
//  edf
//
//  Created by Manuel Cabrerizo on 28/06/2024.
//

#include "edf_level.h"
#include "sys/edf_render_sys.h"

#include "edf_entity.h"
#include "edf_memory.h"
#include "edf.h"

void add_screen(GameState *gs, Level *level, i32 screen_index, const char *positions, u32 row_count, u32 total_size) {

    i32 asteroid_position_rows = row_count;
    i32 asteroid_position_cols = total_size/asteroid_position_rows;
    i32 tile_x = VIRTUAL_RES_X / asteroid_position_cols;
    i32 tile_y = VIRTUAL_RES_Y / asteroid_position_rows;

    for(i32 y = 0; y < asteroid_position_rows; ++y) {
        for(i32 x = 0; x < asteroid_position_cols; ++x) {
            
            i32 screen_x = (x-asteroid_position_rows/2) * tile_x + level->dim.min.x + (VIRTUAL_RES_X * screen_index);
            i32 screen_y = (y-asteroid_position_rows/2) * tile_y;

            if(positions[y*asteroid_position_cols+x] == '*') {
                
                V2 scale = {0};
                Texture texure = 0;

                if(y == 0) {
                    
                    scale = v2((f32)VIRTUAL_RES_X/(f32)asteroid_position_cols, (f32)VIRTUAL_RES_Y/(f32)asteroid_position_rows);

                    if(x == 0) {
                        texure = gs->rocks_corner_texture;
                        scale.x = -scale.x;
                    } else if(x == asteroid_position_cols-1) {
                        texure = gs->rocks_corner_texture;
                    } else {
                        texure = gs->rocks_texutre;
                    }

                    
                } else if(y == asteroid_position_rows-1) {
                    scale = v2((f32)VIRTUAL_RES_X/(f32)asteroid_position_cols, -(f32)VIRTUAL_RES_Y/(f32)asteroid_position_rows);

                    if(x == 0) {
                        texure = gs->rocks_corner_texture;
                        scale.x = -scale.x;
                    } else if(x == asteroid_position_cols-1) {
                        texure = gs->rocks_corner_texture;
                    } else {
                        texure = gs->rocks_texutre;
                    }

                } else {
                    texure = gs->meteorito_texture;
                    scale = v2(200, 200);
                }

                assert(texure);

                V3 pos = v3(screen_x, screen_y, 0);
                Entity *asteroid = entity_manager_add_entity(level->em);
                entity_add_render_component(asteroid, pos, scale, texure, v4(1, 1, 1, 1));

                Collision asteriod_collision;
                asteriod_collision.type = COLLISION_TYPE_CIRLCE;
                asteriod_collision.circle.c = asteroid->pos.xy;
                asteriod_collision.circle.r = asteroid->scale.x*0.5f;
                entity_add_collision_component(asteroid, asteriod_collision, false);
            }
        }
    }

}

Level *load_level(GameState *gs, struct Arena *arena, struct EntityManager *em) {
    Level *level = (Level *)arena_push(arena, sizeof(*level), 8);
    level->arena = arena;
    level->em    = em;
    level->gs    = gs;

    f32 level_horizontal_size = VIRTUAL_RES_X * 20;
    
    level->dim = r2_from_wh(-level_horizontal_size*0.5, -VIRTUAL_RES_Y*0.5f, level_horizontal_size, VIRTUAL_RES_Y);
    level->camera_vel = v3(300, 0, 0);
    level->camera_pos.x = level->dim.min.x + VIRTUAL_RES_X*0.5f;

    {
        static char asteroids[] = {
            "***************"
            "..............."
            "..............."
            "..............."
            "..............."
            "..............."
            "***************"
        };

        add_screen(gs, level, 0, asteroids, 7, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "***************"
            "..............."
            "..............."
            "..............."
            "..............."
            "..............."
            "***************"
        };

        add_screen(gs, level, 1, asteroids, 7, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "...*..........."
            "....*.........."
            ".....*........."
            "......*........"
            ".......*......."
            "..............."
            "..............."
        };

        add_screen(gs, level, 2, asteroids, 7, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "..............."
            "..............."
            ".......*......."
            "......*........"
            ".....*........."
            "....*.........."
            "...*..........."
        };

        add_screen(gs, level, 3, asteroids, 7, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "***************"
            "..............."
            "...*...*....*.."
            "..............."
            ".......*......."
            "...........*..."
            "***************"
        };

        add_screen(gs, level, 4, asteroids, 7, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "..............."
            "..............."
            "..............."
            "..............."
            "..............."
            "..............."
            "..............."
        };

        add_screen(gs, level, 5, asteroids, 7, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "***************"
            "..............."
            "..............."
            "..***********.."
            "..............."
            "..............."
            "..............."
        };

        add_screen(gs, level, 6, asteroids, 7, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "..............."
            "..............."
            "......*..*....."
            ".......**......"
            "......*..*....."
            "..............."
            "***************"
        };

        add_screen(gs, level, 7, asteroids, 7, array_len(asteroids));
    }
    
    

    return level;
}

void level_update(Level *level, f32 dt) {
    level->camera_pos = v3_add(level->camera_pos, v3_scale(level->camera_vel, dt));
    if(level->camera_pos.x + VIRTUAL_RES_X*0.5f >= level->dim.max.x) {
        level->camera_pos.x = level->dim.max.x - VIRTUAL_RES_X*0.5f;
    }
}

void level_render(Level *level, Gpu gpu) {
    render_system_update(level->gs, level->em);
}
