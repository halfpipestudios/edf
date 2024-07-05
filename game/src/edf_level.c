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
#include "edf_asset.h"
#include "edf.h"
#include "edf_particles.h"

static inline void add_wall(Level *level, f32 screen_x, f32 screen_y, V2 scale, Texture texture) {
    V3 pos = v3(screen_x, screen_y, 0);
    Entity *asteroid = entity_manager_add_entity(level->em);
    entity_add_render_component(asteroid, pos, scale, texture, v4(1, 1, 1, 1));

    Collision asteriod_collision = {0};
    asteriod_collision.type = COLLISION_TYPE_AABB;
    asteriod_collision.aabb.min = v2(asteroid->pos.x - fabsf(asteroid->scale.x)*0.45f, asteroid->pos.y - fabsf(asteroid->scale.y)*0.45f);
    asteriod_collision.aabb.max = v2(asteroid->pos.x + fabsf(asteroid->scale.x)*0.45f, asteroid->pos.y + fabsf(asteroid->scale.y)*0.45f);
    entity_add_collision_component(asteroid, asteriod_collision, false);
}

static inline void add_entity(Level *level, f32 screen_x, f32 screen_y, V2 scale, Texture texture) {
    V3 pos = v3(screen_x, screen_y, 0);
    Entity *asteroid = entity_manager_add_entity(level->em);
    entity_add_render_component(asteroid, pos, scale, texture, v4(1, 1, 1, 1));

    V2 dir = v2_normalized(scale);

    Collision asteriod_collision = {0};
    asteriod_collision.type = COLLISION_TYPE_CIRLCE;
    asteriod_collision.offset = v2_scale(dir, -0.15);
    asteriod_collision.circle.c = asteroid->pos.xy;
    asteriod_collision.circle.r = fabsf(asteroid->scale.x)*0.5f;
    entity_add_collision_component(asteroid, asteriod_collision, false);
}

static inline void add_asteroid(Level *level, f32 screen_x, f32 screen_y, V2 scale, Texture texture) {
    V3 pos = v3(screen_x, screen_y, 0);
    Entity *asteroid = entity_manager_add_entity(level->em);
    entity_add_render_component(asteroid, pos, scale, texture, v4(1, 1, 1, 1));

    V2 dir = v2_normalized(scale);

    Collision asteriod_collision = {0};
    asteriod_collision.type = COLLISION_TYPE_CIRLCE;
    asteriod_collision.circle.c = asteroid->pos.xy;
    asteriod_collision.circle.r = fabsf(asteroid->scale.x)*0.5f;
    entity_add_collision_component(asteroid, asteriod_collision, false);
}

void add_screen(GameState *gs, Level *level, i32 screen_index, const char *positions, u32 row_count, u32 total_size) {

    i32 asteroid_position_rows = (i32)row_count;
    i32 asteroid_position_cols = (i32)total_size/asteroid_position_rows;
    f32 tile_x = MAP_COORDS_X / (f32)asteroid_position_cols;
    f32 tile_y = MAP_COORDS_Y / (f32)asteroid_position_rows;
    static Entity *last_trigger = 0;
    for(i32 y = 0; y < asteroid_position_rows; ++y) {
        for(i32 x = 0; x < asteroid_position_cols; ++x) {

            
            f32 map_x = (f32)level->dim.min.x + (MAP_COORDS_X * (f32)screen_index) + tile_x*(f32)x + tile_x*0.5f;
            f32 map_y = (f32)level->dim.min.y + tile_y*(f32)y + tile_y*0.5f;
            
            i32 test_y = (asteroid_position_rows - 1) - y;
            char c = positions[test_y*asteroid_position_cols+x];
            
            V2 scale = v2((f32)MAP_COORDS_X/(f32)asteroid_position_cols, (f32)MAP_COORDS_Y/(f32)asteroid_position_rows);
            Texture texture = 0;

            switch (c) {
                case 't': {
                    texture = am_get_texture(gs->am, "rocks_flat.png");
                    scale.y = -scale.y;
                    add_wall(level, map_x, map_y, scale, texture);
                } break;
                case 'b': {
                    texture = am_get_texture(gs->am, "rocks_flat.png");
                    add_wall(level, map_x, map_y, scale, texture);
                } break;
                case 'q': {
                    texture = am_get_texture(gs->am, "rocks_corner.png");
                    scale.x = -scale.x;
                    scale.y = -scale.y;
                    add_entity(level, map_x, map_y, scale, texture);
                } break;
                case 'e': {
                    texture = am_get_texture(gs->am, "rocks_corner.png");
                    scale.y = -scale.y;
                    add_entity(level, map_x, map_y, scale, texture);
                } break;
                case 'a': {
                    texture = am_get_texture(gs->am, "rocks_corner.png");
                    scale.x = -scale.x;
                    add_entity(level, map_x, map_y, scale, texture);
                } break;
                case 'd': {
                    texture = am_get_texture(gs->am, "rocks_corner.png");
                    add_entity(level, map_x, map_y, scale, texture);
                } break;
                case 'f': {
                    texture = am_get_texture(gs->am, "rock_full.png");
                    add_wall(level, map_x, map_y, scale, texture);
                } break;
                case 'x': {
                    texture = am_get_texture(gs->am, "Meteorito.png");
                    add_asteroid(level, map_x, map_y, scale, texture);
                } break;
                case '!': {
                    texture = am_get_texture(gs->am, "rocks_flat.png");
                    V3 pos = v3(map_x, map_y, 0);
                    last_trigger = entity_manager_add_entity(level->em);
                    entity_add_render_component(last_trigger, pos, scale, texture, v4(1, 1, 1, 0));
                    Collision collision = {0};
                    collision.type = COLLISION_TYPE_AABB;
                    V2 posv2 = v2(map_x, map_y);
                    collision.aabb.min = v2_sub(posv2, v2(1, MAP_COORDS_X*0.5f));
                    collision.aabb.max = v2_add(posv2, v2(1, MAP_COORDS_Y*0.5f));
                    entity_add_collision_component(last_trigger, collision, false);
                    entity_add_trigger_component(last_trigger);
                } break;
                case '*': {
                    texture = am_get_texture(gs->am, "Meteorito.png");
                    V3 pos = v3(map_x, map_y, 0);
                    Entity *asteroid = entity_manager_add_entity(level->em);
                    asteroid->save_pos = pos;
                    entity_add_render_component(asteroid, pos, scale, texture, v4(1, 1, 1, 1));
                    Collision asteriod_collision = {0};
                    asteriod_collision.type = COLLISION_TYPE_CIRLCE;
                    asteriod_collision.circle.c = asteroid->pos.xy;
                    asteriod_collision.circle.r = fabsf(asteroid->scale.x)*0.5f;
                    entity_add_collision_component(asteroid, asteriod_collision, false);
                    entity_add_asteroid_component(asteroid, v2(-4, 0), last_trigger);
                } break;
                default: {
                    
                } break;
            }
        }
    }
}

Level *load_level(GameState *gs, struct Arena *arena, struct EntityManager *em) {
    Level *level = (Level *)arena_push(arena, sizeof(*level), 8);
    level->arena = arena;
    level->em    = em;
    level->gs    = gs;

    f32 level_horizontal_size = MAP_COORDS_X * 20;
    
    level->dim = r2f_from_wh(-level_horizontal_size*0.5f, -MAP_COORDS_Y*0.5f, level_horizontal_size, MAP_COORDS_Y);
    level->camera_vel = v3(2.0f, 0, 0);
    level->camera_pos.x = level->dim.min.x + MAP_COORDS_X*0.5f;

    u32 ship_rand_texture = rand_range(0, 1);
    V3 hero_position = v3((f32)level->dim.min.x+MAP_COORDS_X*0.5f, 0, 0);
    gs->hero = entity_manager_add_entity(gs->em);
    entity_add_input_component(gs->hero);
    entity_add_render_component(gs->hero, hero_position, v2(0.5, 0.5), gs->ship_texture[ship_rand_texture], v4(1, 1, 1, 1));
    entity_add_physics_component(gs->hero, v2(0, 0), v2(0, 0), 0.4f);
    Collision hero_collision = {0};
    hero_collision.type = COLLISION_TYPE_CIRLCE;
    hero_collision.circle.c = gs->hero->pos.xy;
    hero_collision.circle.r = gs->hero->scale.x*0.4f;
    entity_add_collision_component(gs->hero, hero_collision, true);
    entity_add_animation_component(gs->hero, gs->explotion_anim);

    {
        static char asteroids[] = {
            "..............."
            "x..x..........."
            "..............."
            "..............."
            "..............."
            "...........x..."
            "...x.....x....."
            "..............."
        };

        add_screen(gs, level, 0, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "qtttttttttttttt"
            "..............."
            "...........x..."
            ".....x........."
            "..............."
            "..........x...."
            "..............."
            "abbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 1, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..........x...."
            "..............."
            "abbbbbbbbbbbbbd"
            "qttttttttttttte"
            "..............."
            "...x..........."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 2, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            ".......ad......"
            "..abbbbffbbbd.."
            "..qttttffttte.."
            ".......qe......"
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 3, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "..............."
            ".....abbbd....."
            "....afffffd...."
            "...afffffffd..."
            "..afffffffffd.."
            "bbfffffffffffbb"
        };

        add_screen(gs, level, 4, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttffffffffffftt"
            "..qfffffffffe.."
            "...qfffffffe..."
            "....qfffffe...."
            ".....qfffe....."
            "..............."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 5, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "tttttttttttffff"
            "...........qfff"
            "............qff"
            ".............qf"
            "..............q"
            "..............."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 6, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ffffttttttttttt"
            "fffe..........."
            "ffe............"
            "fe..ad........."
            "e..affd........"
            "..affffd......."
            ".affffffd......"
            "bffffffffbbbbbb"
        };

        add_screen(gs, level, 7, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "..............."
            "!.............."
            "..............."
            "..............."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 8, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "..........*...."
            "..............."
            "..........*...."
            "..............."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 9, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............*"
            "..............."
            "..............*"
            "..............."
            "..............*"
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 10, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "..............*"
            "..............."
            "..............*"
            "..............."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 11, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "..............."
            ".......x......."
            "..............."
            "..........x...."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 12, asteroids, 8, array_len(asteroids));
    }

#if 1
    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "....x.........."
            "..............."
            "..............."
            "...x..........."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 13, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "......x........"
            "...........x..."
            "..............."
            ".......x......."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 14, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            ".......x......."
            "..............."
            "..............."
            ".......x......."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 15, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "..............."
            "..x.........x.."
            "..............."
            "..............."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 16, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            ".......x......."
            "..............."
            ".......x......."
            "..x.......x...."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 17, asteroids, 8, array_len(asteroids));
    }   

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "..........x...."
            "..............."
            "..............."
            "..x............"
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 18, asteroids, 8, array_len(asteroids));
    }

    {
        static char asteroids[] = {
            "ttttttttttttttt"
            "..............."
            "...x..........."
            "..............."
            "..............."
            "......x.x.x...."
            "..............."
            "bbbbbbbbbbbbbbb"
        };

        add_screen(gs, level, 19, asteroids, 8, array_len(asteroids));
    }

#endif

    return level;
}

void level_update(Level *level, f32 dt) {
    level->camera_pos = v3_add(level->camera_pos, v3_scale(level->camera_vel, dt));
    if(level->camera_pos.x >= level->dim.max.x - MAP_COORDS_X*0.5f) {
        level->camera_pos.x = level->dim.max.x - MAP_COORDS_X*0.5f;
        level->camera_vel = v3(0, 0, 0);
    }
    


    f32 radii = level->gs->hero->collision.circle.r*2.0f;
    AABB camera_bound = {};
    camera_bound.min = v2(level->camera_pos.x - MAP_COORDS_X*0.5f + radii, level->camera_pos.y - MAP_COORDS_Y*0.5f + radii);
    camera_bound.max = v2(level->camera_pos.x + MAP_COORDS_X*0.5f - radii, level->camera_pos.y + MAP_COORDS_Y*0.5f- radii);
    if(test_circle_aabb(level->gs->hero->collision.circle, camera_bound) == false) {
        
        if(level->gs->hero->animation->playing == false) {
            level->gs->hero->save_tex = level->gs->hero->tex;
            level->gs->hero->animation->playing = true;
        }
        level->gs->hero->vel = v2(0, 0);
        level->gs->hero->acc = v2(0, 0);
        particle_system_stop(level->gs->ps);
        level->camera_vel = v3(0, 0, 0);
    }
        
    

    
}

void level_render(Level *level, Gpu gpu) {
}
