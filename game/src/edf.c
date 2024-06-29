#include "edf.h"
#include "edf_entity.h"
#include "sys/edf_render_sys.h"
#include "sys/edf_input_sys.h"
#include "sys/edf_physics_sys.h"
#include "sys/edf_collision_sys.h"
#include "sys/edf_animation_sys.h"
#include "sys/edf_enemy_sys.h"
#include "edf_level.h"
#include "edf_efx.h"

#include <stdarg.h>
#include <stdio.h>

Console *gcs;

void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);

    init_scratch_arenas(memory, 3, mb(10));

    gs->platform_arena = arena_create(memory, mb(20));
    gs->game_arena     = arena_create(memory, mb(50));

    gs->gpu   = gpu_load(&gs->platform_arena);
    gs->spu   = spu_load(&gs->platform_arena);
    gs->liberation = font_load(gs->gpu, &gs->platform_arena, "LiberationMono-Regular.ttf", 32);
    gs->times = font_load(gs->gpu, &gs->platform_arena, "times.ttf", 64);

    gs->cs = cs_init(gs->liberation, -VIRTUAL_RES_X/2, -40, 600, VIRTUAL_RES_Y/2);
    gcs = &gs->cs;

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
    gs->meteorito_bitmap  = bitmap_load(&gs->game_arena, "Meteorito.png");
    gs->deathstar_bitmap  = bitmap_load(&gs->game_arena, "deathstar.png");
    gs->orbe_bitmap       = bitmap_load(&gs->game_arena, "orbe_shot.png");
    gs->confeti_bitmap[0] = bitmap_load(&gs->game_arena, "confeti1.png");
    gs->confeti_bitmap[1] = bitmap_load(&gs->game_arena, "confeti2.png");
    gs->confeti_bitmap[2] = bitmap_load(&gs->game_arena, "confeti3.png");
    gs->confeti_bitmap[3] = bitmap_load(&gs->game_arena, "confeti4.png");
    gs->confeti_bitmap[4] = bitmap_load(&gs->game_arena, "confeti5.png");
    gs->pause_bitmap      = bitmap_load(&gs->game_arena, "pause.png");
    gs->rocks_bitmap      = bitmap_load(&gs->game_arena, "rocks_flat.png");
    gs->rocks_full_bitmap = bitmap_load(&gs->game_arena, "rock_full.png");
    gs->rocks_corner_bitmap = bitmap_load(&gs->game_arena, "rocks_corner.png");
    gs->square_bitmap     = bitmap_load(&gs->game_arena, "square.png");
    static char explotion_name_buffer[256];
    for(u32 i = 0; i < array_len(gs->explotion_bitmaps); ++i) {
        sprintf(explotion_name_buffer, "destroyed/destroyed%d.png", (i+1));
        gs->explotion_bitmaps[i] = bitmap_load(&gs->game_arena, explotion_name_buffer);
        
    }

    gs->ship_texture[0]      = gpu_texture_load(gs->gpu, &gs->ship_bitmap[0]);
    gs->ship_texture[1]      = gpu_texture_load(gs->gpu, &gs->ship_bitmap[1]);
    gs->move_outer_texture   = gpu_texture_load(gs->gpu, &gs->move_outer_bitmap);
    gs->move_inner_texture   = gpu_texture_load(gs->gpu, &gs->move_inner_bitmap);
    gs->boost_texture        = gpu_texture_load(gs->gpu, &gs->boost_bitmap);
    gs->star_texture         = gpu_texture_load(gs->gpu, &gs->star_bitmap);
    gs->galaxy_texture       = gpu_texture_load(gs->gpu, &gs->galaxy_bitmap);
    gs->planet1_texture      = gpu_texture_load(gs->gpu, &gs->planet1_bitmap);
    gs->planet2_texture      = gpu_texture_load(gs->gpu, &gs->planet2_bitmap);
    gs->satelite_texture     = gpu_texture_load(gs->gpu, &gs->satelite_bitmap);
    gs->meteorito_texture    = gpu_texture_load(gs->gpu, &gs->meteorito_bitmap);
    gs->deathstar_texture    = gpu_texture_load(gs->gpu, &gs->deathstar_bitmap);
    gs->orbe_texture         = gpu_texture_load(gs->gpu, &gs->orbe_bitmap);
    gs->confeti_texture[0]   = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[0]);
    gs->confeti_texture[1]   = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[1]);
    gs->confeti_texture[2]   = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[2]);
    gs->confeti_texture[3]   = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[3]);
    gs->confeti_texture[4]   = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[4]);
    gs->pause_texture        = gpu_texture_load(gs->gpu, &gs->pause_bitmap);
    gs->rocks_texutre        = gpu_texture_load(gs->gpu, &gs->rocks_bitmap);
    gs->rocks_full_texture   = gpu_texture_load(gs->gpu, &gs->rocks_full_bitmap);
    gs->rocks_corner_texture = gpu_texture_load(gs->gpu, &gs->rocks_corner_bitmap);
    gs->square_texture       = gpu_texture_load(gs->gpu, &gs->square_bitmap);
    for(u32 i = 0; i < array_len(gs->explotion_textures); ++i) {
        gs->explotion_textures[i] = gpu_texture_load(gs->gpu, &gs->explotion_bitmaps[i]);
        
    }

    gs->explotion_anim = animation_load(&gs->game_arena,
                           gs->explotion_textures, array_len(gs->explotion_textures),
                           0.1f, false, false);


    gs->em = entity_manager_load(&gs->game_arena, 1000);

    gs->level = load_level(gs, &gs->game_arena, gs->em);

    stars_init(gs);
    
    gs->fire    = particle_system_create(&gs->game_arena, 100, 10,
                                         0.05f, v2(0, 0), gs->orbe_texture,
                                         ship_ps_update, GPU_BLEND_STATE_ADDITIVE);
    gs->confeti = particle_system_create(&gs->game_arena, 1000, 10,
                                         0.05f, v2(0, 0), 0,
                                         confeti_ps_update, GPU_BLEND_STATE_ALPHA);
    gs->neon    = particle_system_create(&gs->game_arena, 200, 20,
                                         0.05f, v2(0, 0), gs->orbe_texture,
                                         neon_ps_update, GPU_BLEND_STATE_ADDITIVE);
    gs->smoke    = particle_system_create(&gs->game_arena, 1000, 5,
                                         0.05f, v2(0, 0), gs->star_texture,
                                         smoke_ps_update, GPU_BLEND_STATE_ALPHA);
    gs->pixel    = particle_system_create(&gs->game_arena, 100, 10,
                                          0.05f, v2(0, 0), gs->square_texture,
                                          pixel_ps_update, GPU_BLEND_STATE_ADDITIVE);

    gs->ps = gs->fire;

    /*
    u32 ship_rand_texture = rand_range(0, 1);

    V3 hero_position = v3((f32)gs->level->dim.min.x, 0, 0);
    gs->hero = entity_manager_add_entity(gs->em);
    entity_add_input_component(gs->hero);
    entity_add_render_component(gs->hero, hero_position, v2(32*3, 32*3), gs->ship_texture[ship_rand_texture], v4(1, 1, 1, 1));
    entity_add_physics_component(gs->hero, v2(0, 0), v2(0, 0), 0.4f);
    Collision hero_collision;
    hero_collision.type = COLLISION_TYPE_CIRLCE;
    hero_collision.circle.c = gs->hero->pos.xy;
    hero_collision.circle.r = gs->hero->scale.x*0.4f;
    entity_add_collision_component(gs->hero, hero_collision, true);

    entity_add_animation_component(gs->hero, &gs->game_arena,
        gs->explotion_textures, array_len(gs->explotion_textures), 0.1f, false, false);
    */

    i32 hw = VIRTUAL_RES_X * 0.5f;
    i32 hh = VIRTUAL_RES_Y * 0.5f;
    R2 window_rect;
    window_rect.min.x = -hw;
    window_rect.max.x = 0;
    window_rect.min.y = -hh;
    window_rect.max.y = hh;
    gs->joystick = ui_joystick_alloc(&gs->ui, &gs->game_arena, v2(-740, -250), window_rect, 
                                    140, 220, gs->move_inner_texture, gs->move_outer_texture, v4(1,1,1,0.3f));

    gs->boost_button = ui_button_alloc(&gs->ui, &gs->game_arena, v2(740, -250), 135, gs->boost_texture, v4(1,1,1,0.3f));

    f32 paddin_top = 80;
    f32 pause_buttom_dim = 140;
    V2 pause_button_pos = v2(0, VIRTUAL_RES_Y*0.5f-pause_buttom_dim*0.5f-paddin_top);
    gs->pause_button = ui_button_alloc(&gs->ui, &gs->game_arena, pause_button_pos, pause_buttom_dim*0.5f, gs->pause_texture, v4(1,0,0,0.3f));
    pause_button_pos.x += 220;
    gs->next_ship_button = ui_button_alloc(&gs->ui, &gs->game_arena, pause_button_pos, pause_buttom_dim*0.5f, gs->pause_texture, v4(0,1,0,0.3f));
    pause_button_pos.x += 220;
    gs->next_boost_button = ui_button_alloc(&gs->ui, &gs->game_arena, pause_button_pos, pause_buttom_dim*0.5f, gs->pause_texture, v4(0,0,1,0.3f));
    pause_button_pos.x += 220;
    gs->debug_button = ui_button_alloc(&gs->ui, &gs->game_arena, pause_button_pos, pause_buttom_dim*0.5f, gs->pause_texture, v4(1,0,1,0.3f));

}

void game_update(Memory *memory, Input *input, f32 dt) {
    
    GameState *gs = game_state(memory);

    ui_begin(&gs->ui, &gs->mt, input, dt);

    if(ui_button_just_up(&gs->mt, gs->pause_button)) {
        gs->paused = !gs->paused;
    }

    if(ui_button_just_up(&gs->mt, gs->debug_button)) {
        gs->debug_show = !gs->debug_show;
    }

    if(ui_button_just_up(&gs->mt, gs->next_ship_button)) {
        static u32 next_ship = 0;
        if(gs->hero->tex == gs->ship_texture[next_ship]) {
            next_ship = (next_ship + 1) % array_len(gs->ship_texture);
        }
        gs->hero->tex = gs->ship_texture[next_ship];
        next_ship = (next_ship + 1) % array_len(gs->ship_texture);
    }

    if(ui_button_just_up(&gs->mt, gs->next_boost_button)) {

        ParticleSystem *particles[] = {
            gs->fire,
            gs->confeti,
            gs->neon,
            gs->smoke,
            gs->pixel
        };

        static u32 next = 0;

        particle_system_stop(gs->ps);
        if(gs->ps == particles[next]) {
            next = (next + 1) % array_len(particles);
        }
        gs->ps = particles[next];
        next = (next + 1) % array_len(particles);
        particle_system_reset(gs->ps);
        particle_system_start(gs->ps);
    }

    if(!gs->paused) {
        input_system_update(gs, gs->em, dt);
        physics_system_update(gs->em, dt);
        collision_system_update(gs, gs->em, dt);
        animation_system_update(gs, gs->em, dt);
        trigger_system_update(gs, gs->em, dt);
        asteroid_system_update(gs->em, dt);
        stars_update(gs, dt);
        level_update(gs->level, dt);

        V2 dir;
        dir.x = cosf(gs->hero->angle + PI*0.5f);
        dir.y = sinf(gs->hero->angle + PI*0.5f);
        particle_system_set_position(gs->ps, v2_sub(v2(gs->hero->pos.x, gs->hero->pos.y), v2_scale(dir, gs->hero->scale.y*0.5f)));
        
        particle_system_update(gs, gs->ps, dt);
    }

   ui_end(&gs->ui, &gs->mt, input);

    // FPS Counter
    gs->fps_counter += 1;
    gs->time_per_frame += dt;
    if(gs->time_per_frame >= 1) {
        gs->FPS = gs->fps_counter;
        gs->fps_counter = 0;
        gs->time_per_frame = gs->time_per_frame - 1.0f;
    }
}


void game_render(Memory *memory) {
    GameState *gs = game_state(memory);

    gpu_frame_begin(gs->gpu);

    // render the back ground
    i32 w = (i32)VIRTUAL_RES_X;
    i32 h = (i32)VIRTUAL_RES_Y;
    gpu_camera_set(gs->gpu, v3(0, 0, 0), 0);
    gpu_draw_quad_color(gs->gpu, 0, 0, w, h, 0, v4(0.05f, 0.05f, 0.1f, 1));

    // Entities draw
    gpu_camera_set(gs->gpu, gs->level->camera_pos, 0);

    stars_render(gs);
    particle_system_render(gs->gpu, gs->ps);
    render_system_update(gs, gs->em);

    // UI draw
    gpu_camera_set(gs->gpu, v3(0, 0, 0), 0);
    ui_render(gs->gpu, &gs->ui);

    if(gs->debug_show) {
        cs_render(gs->gpu, &gs->cs);
        static char fps_text[1024];
        snprintf(fps_text, 1024, "FPS: %d", gs->FPS);
        R2 dim = font_size_text(gs->liberation, fps_text);
        f32 pos_x = -VIRTUAL_RES_X*0.5f;
        f32 pos_y = VIRTUAL_RES_Y*0.5f - r2_height(dim);
        font_draw_text(gs->gpu, gs->liberation, fps_text, pos_x, pos_y, v4(1, 1, 1, 1));
    }

    
    if(gs->paused) {
        char *text = "Pause";
        R2 dim = font_size_text(gs->times, text);
        f32 pos_x = 0 - r2_width(dim)*0.5f;
        f32 pos_y = 0 - r2_height(dim)*0.5f + VIRTUAL_RES_Y*0.25f;
        font_draw_text(gs->gpu, gs->times, text, pos_x, pos_y, v4(1, 1, 1, 1));
    }

    gpu_frame_end(gs->gpu);
}

void game_shutdown(Memory *memory) {
    GameState *gs = game_state(memory);
    gpu_unload(gs->gpu);
}

void game_resize(Memory *memory, u32 w, u32 h) {
    GameState *gs = game_state(memory);
    unused(w); unused(h);
    gpu_resize(gs->gpu, VIRTUAL_RES_X, VIRTUAL_RES_Y);
}
