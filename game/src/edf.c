#include "edf.h"
#include "edf_entity.h"
#include "sys/edf_render_sys.h"
#include "sys/edf_input_sys.h"
#include "sys/edf_physics_sys.h"

#include <stdlib.h> 
#include <stdarg.h>
#include <stdio.h>
#include <time.h>


i32 rand_range(i32 min, i32 max) {
    return (rand() % (max - min + 1)) + min;
}

void stars_init(GameState *gs) {
    static u32 star_colors[4] = {
        0xFDF4F5,
        0xE8A0BF,
        0xBA90C6,
        0xC0DBEA
    };

    i32 hw = (os_display_width() * 0.5f) * 1.25f;
    i32 hh = (os_display_height() * 0.5f) * 1.25f;
    
    Texture planet_textures[MAX_GALAXY] = {
        gs->galaxy_texture,
        gs->planet1_texture,
        gs->planet2_texture,
        gs->satelite_texture,
        gs->meteorito_texture,
        gs->deathstar_texture
    };


    i32 color_index = 0;
    srand(123);
    for(i32 i = 0; i < MAX_GALAXY; i++) {
        Sprite *galaxy = gs->galaxy + i;
        galaxy->texture = planet_textures[color_index];
        galaxy->pos.x = rand_range(-hw, hw);
        galaxy->pos.y = rand_range(-hh, hh);
        galaxy->z = rand_range(2, 10);
        f32 ratio = ((f32)rand_range(50, 100) / 100.0f);
        galaxy->scale = v2(200*ratio, 200*ratio);
        galaxy->tint = v3(0.4f, 0.4f, 0.4f);
        galaxy->angle = 0;
        color_index++;
    }

    // Init star sprites
    color_index = 0;
    for(i32 i = 0; i < MAX_STARS; i++) {
        Sprite *star = gs->stars + i;
        star->texture = gs->star_texture;
        star->pos.x = rand_range(-hw, hw);
        star->pos.y = rand_range(-hh, hh);
        star->z = (f32)rand_range(1, 10);
        star->scale.x = 10/star->z;
        star->scale.y = 10/star->z;
        star->tint = hex_to_v3(star_colors[color_index]);
        star->angle = 0;
        
        color_index = (color_index + 1) % array_len(star_colors);
    }
}

void stars_update(GameState *gs, f32 dt) {
    i32 hw = os_display_width() * 0.5f;
    i32 hh = os_display_height() * 0.5f;

    R2 bounds = {0};
    bounds.min.x = gs->hero->pos.x - (hw * 1.25f);
    bounds.min.y = gs->hero->pos.y - (hh * 1.25f);
    bounds.max.x = gs->hero->pos.x + (hw * 1.25f);
    bounds.max.y = gs->hero->pos.y + (hh * 1.25f);
    for(i32 i = 0; i < MAX_GALAXY; i++) {
        Sprite *galaxy = gs->galaxy + i;
        galaxy->pos.x -= (gs->hero->vel.x / galaxy->z) * dt;
        galaxy->pos.y -= (gs->hero->vel.y / galaxy->z) * dt;
        
        if(galaxy->pos.x > bounds.max.x) {
            galaxy->pos.x = bounds.min.x;
        }
        if(galaxy->pos.x < bounds.min.x) {
            galaxy->pos.x = bounds.max.x;
        }

        if(galaxy->pos.y > bounds.max.y) {
            galaxy->pos.y = bounds.min.y;
        }
        if(galaxy->pos.y < bounds.min.y) {
            galaxy->pos.y = bounds.max.y;
        }
    }


    for(i32 i = 0; i < MAX_STARS; i++) {
        Sprite *star = gs->stars + i;
        star->pos.x -= (gs->hero->vel.x / star->z) * dt;
        star->pos.y -= (gs->hero->vel.y / star->z) * dt;
        
        if(star->pos.x > bounds.max.x) {
            star->pos.x = bounds.min.x;
        }
        if(star->pos.x < bounds.min.x) {
            star->pos.x = bounds.max.x;
        }

        if(star->pos.y > bounds.max.y) {
            star->pos.y = bounds.min.y;
        }
        if(star->pos.y < bounds.min.y) {
            star->pos.y = bounds.max.y;
        }
    }
}

void stars_render(GameState *gs) {
    // draw the planets
    for(i32 i = 0; i < MAX_GALAXY; i++) {
        Sprite *galaxy = gs->galaxy + i;
        sprite_draw(gs->gpu, galaxy);
    }

    // draw the stars
    for(i32 i = 0; i < MAX_STARS; i++) {
        Sprite *star = gs->stars + i;
        sprite_draw(gs->gpu, star);
    }
}

void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);

    init_scratch_arenas(memory, 3, mb(10));

    gs->platform_arena = arena_create(memory, mb(20));
    gs->game_arena     = arena_create(memory, mb(50));

    gs->gpu   = gpu_load(&gs->platform_arena);
    gs->spu   = spu_load(&gs->platform_arena);
    gs->arial = font_load(gs->gpu, &gs->platform_arena, "arial.ttf", 128);
    gs->times = font_load(gs->gpu, &gs->platform_arena, "times.ttf", 64);

    gs->ship_bitmap[0]    = bitmap_load(&gs->game_arena, "Player.png");
    gs->ship_bitmap[1]    = bitmap_load(&gs->game_arena, "OG Es.png");
    gs->move_outer_bitmap = bitmap_load(&gs->game_arena, "move_outer.png");
    gs->move_inner_bitmap = bitmap_load(&gs->game_arena, "move_inner.png");
    gs->boost_bitmap      = bitmap_load(&gs->game_arena, "boost.png");
    gs->star_bitmap       = bitmap_load(&gs->game_arena, "star.png");
    gs->galaxy_bitmap     = bitmap_load(&gs->game_arena, "stb_image.png");
    gs->planet1_bitmap    = bitmap_load(&gs->game_arena, "planet1.png");
    gs->planet2_bitmap    = bitmap_load(&gs->game_arena, "planet2.png");
    gs->satelite_bitmap   = bitmap_load(&gs->game_arena, "Satelite.png");
    gs->meteorito_bitmap   = bitmap_load(&gs->game_arena, "Meteorito.png");
    gs->deathstar_bitmap   = bitmap_load(&gs->game_arena, "deathstar.png");

    gs->ship_texture[0]    = gpu_texture_load(gs->gpu, &gs->ship_bitmap[0]);
    gs->ship_texture[1]    = gpu_texture_load(gs->gpu, &gs->ship_bitmap[1]);
    gs->move_outer_texture = gpu_texture_load(gs->gpu, &gs->move_outer_bitmap);
    gs->move_inner_texture = gpu_texture_load(gs->gpu, &gs->move_inner_bitmap);
    gs->boost_texture      = gpu_texture_load(gs->gpu, &gs->boost_bitmap);
    gs->star_texture       = gpu_texture_load(gs->gpu, &gs->star_bitmap);
    gs->galaxy_texture     = gpu_texture_load(gs->gpu, &gs->galaxy_bitmap);
    gs->planet1_texture    = gpu_texture_load(gs->gpu, &gs->planet1_bitmap);
    gs->planet2_texture    = gpu_texture_load(gs->gpu, &gs->planet2_bitmap);
    gs->satelite_texture   = gpu_texture_load(gs->gpu, &gs->satelite_bitmap);
    gs->meteorito_texture  = gpu_texture_load(gs->gpu, &gs->meteorito_bitmap);
    gs->deathstar_texture  = gpu_texture_load(gs->gpu, &gs->deathstar_bitmap);

    gs->em = entity_manager_load(&gs->game_arena, 100);

    // hero initialization
    f32 size = 32.0f * 3.0f;
    srand(time(0));
    u32 ship_rand_texture = rand_range(0, 1);
    gs->hero = entity_manager_add_entity(gs->em);
    entity_add_input_component(gs->hero);
    entity_add_render_component(gs->hero, v3(0, 0, 0), v2(size, size), gs->ship_texture[ship_rand_texture], v3(1, 1, 1));
    entity_add_physics_component(gs->hero, v2(0, 0), v2(0, 0), 0.4f);

    gs->game_init = false;
}

void game_update(Memory *memory, Input *input, f32 dt) {
    GameState *gs = game_state(memory);

    if(gs->game_init == false) {
        i32 hw = os_display_width() * 0.5f;
        i32 hh = os_display_height() * 0.5f;
        R2 window_rect;
        
        window_rect.min.x = -hw;
        window_rect.max.x = 0;
        window_rect.min.y = -hh;
        window_rect.max.y = hh;
        gs->joystick = ui_joystick_alloc(&gs->ui, &gs->game_arena, v2(-740, -250), window_rect, 
                                        140, 220, gs->move_inner_texture, gs->move_outer_texture);
        gs->button = ui_button_alloc(&gs->ui, &gs->game_arena, v2(740, -250), 135, gs->boost_texture);
        
        f32 radio = 80;
        R2 rect = { {740-radio, 250-radio}, {740+radio, 250+radio} };
        gs->joystick2 = ui_joystick_alloc(&gs->ui, &gs->game_arena, v2(740, 250), rect, 
                                radio, radio*1.6f, gs->move_inner_texture, gs->move_outer_texture);

        gs->button2 = ui_button_alloc(&gs->ui, &gs->game_arena, v2(340, -250), 100, gs->deathstar_texture);

        stars_init(gs);
        gs->game_init = true;
    }

    mt_begin(&gs->mt, input);
    ui_update(&gs->ui, &gs->mt, dt);
    input_system_update(gs, gs->em, dt);
    mt_end(&gs->mt, input);

    physics_system_update(gs->em, dt);
    stars_update(gs, dt);

    // FPS Counter
    gs->fps_counter += 1;
    gs->time_per_frame += dt;
    if(gs->time_per_frame >= 1) {
        gs->FPS = gs->fps_counter;
        gs->fps_counter = 0;
        gs->time_per_frame = 1.0f - gs->time_per_frame;
    }
}


void game_render(Memory *memory) {
    GameState *gs = game_state(memory);

    gpu_frame_begin(gs->gpu);

    // render the back ground
    i32 w = (i32)os_display_width();
    i32 h = (i32)os_display_height();
    gpu_camera_set(gs->gpu, v3(0, 0, 0), 0);
    gpu_draw_quad_color(gs->gpu, 0, 0, w, h, 0, v3(0.05f, 0.05f, 0.1f));

    // Entities draw
    gpu_camera_set(gs->gpu, v3(gs->hero->pos.x, gs->hero->pos.y, 0), 0);
    stars_render(gs);
    render_system_update(gs, gs->em);

    // UI draw
    gpu_camera_set(gs->gpu, v3(0, 0, 0), 0);
    ui_render(gs->gpu, &gs->ui);

    static char fps_text[1024];
    snprintf(fps_text, 1024, "FPS: %d", gs->FPS);
    font_draw_text(gs->gpu, gs->times, fps_text, -w*0.5f + 150, h*0.5f-150, v3(1, 1, 1));

    gpu_frame_end(gs->gpu);

}

void game_shutdown(Memory *memory) {
    GameState *gs = game_state(memory);
    gpu_unload(gs->gpu);
}

void game_resize(Memory *memory, u32 w, u32 h) {
    GameState *gs = game_state(memory);
    gpu_resize(gs->gpu, w, h);
}
