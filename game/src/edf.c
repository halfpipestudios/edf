#include "edf.h"
#include "edf_entity.h"
#include "edf_level.h"
#include "edf_efx.h"
#include "edf_asset.h"

#include "sys/edf_render_sys.h"
#include "sys/edf_input_sys.h"
#include "sys/edf_physics_sys.h"
#include "sys/edf_collision_sys.h"
#include "sys/edf_animation_sys.h"
#include "sys/edf_enemy_sys.h"


#include <stdarg.h>
#include <stdio.h>

// ------------------------
// Globals
// ------------------------

Console *gcs;
R2 display;
R2 game_view;

// ------------------------
// Game
// ------------------------

#define DEBUG_PADDING_X 120
#define DEBUG_PADDING_Y 40

void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);

    init_scratch_arenas(memory, 3, mb(10));

    // arena_push(get_scratch_arena(0), mb(11), 8);
    gs->platform_arena = arena_create(memory, mb(20));
    gs->game_arena     = arena_create(memory, mb(50));

    u64 platform_arena_end = (u64)gs->platform_arena.data + gs->platform_arena.size;
    u64 game_arena_start = (u64)gs->game_arena.data;
    assert(platform_arena_end <= game_arena_start);

    gs->gpu = gpu_load(&gs->platform_arena);
    gs->spu = spu_load(&gs->platform_arena);
    gs->am  = am_load(&gs->game_arena, gs->gpu);

    i32 debug_y_pos = -DEBUG_PADDING_Y + r2_height(display)/2-60;
    i32 debug_x_pos = DEBUG_PADDING_X + -r2_width(display)/2;
    gs->cs = cs_init(am_get_font(gs->am, "LiberationMono-Regular.ttf", 32), debug_x_pos, debug_y_pos, 600, r2_height(display)/2);
    gcs = &gs->cs;
    gs->av = av_init(&gs->platform_arena, am_get_font(gs->am, "LiberationMono-Regular.ttf", 32), debug_x_pos+620, debug_y_pos, 600);
    av_add_arena(&gs->av, get_scratch_arena(0), "scratch_arena 0");
    av_add_arena(&gs->av, get_scratch_arena(1), "scratch_arena 1");
    av_add_arena(&gs->av, &gs->platform_arena, "platform arena");
    av_add_arena(&gs->av, &gs->game_arena, "game arena");

    gs->em = entity_manager_load(&gs->game_arena, 800);
    
    gs->fire    = particle_system_create(&gs->game_arena, 100, 10,
                                         0.05f, v2(0, 0), am_get_texture(gs->am, "orbe.png"),
                                         ship_ps_update, GPU_BLEND_STATE_ADDITIVE);
    gs->confeti = particle_system_create(&gs->game_arena, 1000, 10,
                                         0.05f, v2(0, 0), 0,
                                         confeti_ps_update, GPU_BLEND_STATE_ALPHA);
    gs->neon    = particle_system_create(&gs->game_arena, 200, 20,
                                         0.05f, v2(0, 0), am_get_texture(gs->am, "orbe.png"),
                                         neon_ps_update, GPU_BLEND_STATE_ADDITIVE);
    gs->smoke   = particle_system_create(&gs->game_arena, 1000, 5,
                                         0.05f, v2(0, 0), am_get_texture(gs->am, "star.png"),
                                         smoke_ps_update, GPU_BLEND_STATE_ALPHA);
    gs->pixel   = particle_system_create(&gs->game_arena, 100, 10,
                                          0.05f, v2(0, 0), am_get_texture(gs->am, "square.png"),
                                          pixel_ps_update, GPU_BLEND_STATE_ADDITIVE);

    gs->ps = gs->fire;

    i32 hw = r2_width(display)/2;
    i32 hh = r2_height(display)/2;
    R2 window_rect;
    window_rect.min.x = -hw;
    window_rect.max.x = 0;
    window_rect.min.y = -hh;
    window_rect.max.y = hh;
    gs->joystick = ui_joystick_alloc(&gs->ui, &gs->game_arena, v2((f32)-hw+220, (f32)-hh+220), window_rect,
                                    140, 220, am_get_texture(gs->am, "move_inner.png"), am_get_texture(gs->am, "move_outer.png"), v4(1,1,1,0.3f));

    gs->boost_button = ui_button_alloc(&gs->ui, &gs->game_arena, v2((f32)hw-135, (f32)-hh+135), 135, am_get_texture(gs->am, "boost.png"), v4(1,1,1,0.3f));

    f32 paddin_top = 80;
    f32 pause_buttom_dim = 140;
    V2 pause_button_pos = v2(pause_buttom_dim*0.5f, (f32)hh-pause_buttom_dim*0.5f-paddin_top);
    gs->pause_button = ui_button_alloc(&gs->ui, &gs->game_arena, pause_button_pos, pause_buttom_dim*0.5f, am_get_texture(gs->am, "pause.png"), v4(1,0,0,0.3f));
    pause_button_pos.x += 220;
    gs->next_ship_button = ui_button_alloc(&gs->ui, &gs->game_arena, pause_button_pos, pause_buttom_dim*0.5f, am_get_texture(gs->am, "pause.png"), v4(0,1,0,0.3f));
    pause_button_pos.x += 220;
    gs->next_boost_button = ui_button_alloc(&gs->ui, &gs->game_arena, pause_button_pos, pause_buttom_dim*0.5f, am_get_texture(gs->am, "pause.png"), v4(0,0,1,0.3f));
    pause_button_pos.x += 220;
    gs->debug_button = ui_button_alloc(&gs->ui, &gs->game_arena, pause_button_pos, pause_buttom_dim*0.5f, am_get_texture(gs->am, "pause.png"), v4(1,0,1,0.3f));

    gs->ship_texture[0] = am_get_texture(gs->am, "Player.png");
    gs->ship_texture[1] = am_get_texture(gs->am, "OG Es.png");

    static char name_buffer[256];
    for(u32 i = 0; i < array_len(gs->explotion_textures); ++i) {
        sprintf(name_buffer, "destroyed/destroyed%d.png", (i+1));
        gs->explotion_textures[i] = am_get_texture(gs->am, name_buffer);
        
    }

    for(u32 i = 0; i < array_len(gs->confeti_texture); ++i) {
        sprintf(name_buffer, "confeti%d.png", (i+1));
        gs->confeti_texture[i] = am_get_texture(gs->am, name_buffer);
        
    }

    gs->explotion_anim = animation_load(&gs->game_arena,
                                        gs->explotion_textures, array_len(gs->explotion_textures),
                                        0.1f, false, false);

    gs->level = load_level(gs, &gs->game_arena, gs->em);

    stars_init(gs);

    cs_print(gcs, "test 1: dhsajhd dsahdjksha dshajkdhs dshkajdshak\n");
    cs_print(gcs, "test 2: ----------------------------------------\n");
    
    gs->render_target = gpu_render_targte_load(gs->gpu, VIRTUAL_RES_X, VIRTUAL_RES_Y);
    
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

    f32 ms = (dt * 1000);
    gs->MS = ms;

    //cs_print(gcs, "asset used: %d\n", gs->am->assets_table_used);
}

void game_render(Memory *memory) {
    GameState *gs = game_state(memory);

    gpu_frame_begin(gs->gpu);

    {
        // render the back ground
        f32 w = MAP_COORDS_X;
        f32 h = MAP_COORDS_Y;

        gpu_render_target_begin(gs->gpu, gs->render_target);

        gpu_viewport_set(gs->gpu, 0, 0, VIRTUAL_RES_X, VIRTUAL_RES_Y);
        gpu_projection_set(gs->gpu, (f32)(-w)*0.5f, (f32)w*0.5f, (f32)h*0.5f, (f32)(-h)*0.5f);

        gpu_camera_set(gs->gpu, v3(0, 0, 0), 0);
        gpu_draw_quad_color(gs->gpu, 0, 0, (f32)w, (f32)h, 0, v4(0.05f, 0.05f, 0.1f, 1));
        
        // Entities draw
        gpu_camera_set(gs->gpu, gs->level->camera_pos, 0);
        stars_render(gs);
        particle_system_render(gs->gpu, gs->ps);
        render_system_update(gs, gs->em);

        gpu_render_target_end(gs->gpu, gs->render_target);
    }

    i32 w = r2_width(display);
    i32 h = r2_height(display);

    gpu_render_target_begin(gs->gpu, 0);

    gpu_viewport_set(gs->gpu, (f32)display.min.x, (f32)display.min.y, (f32)r2_width(display), (f32)r2_height(display));
    gpu_projection_set(gs->gpu, (f32)(-w)*0.5f, (f32)w*0.5f, (f32)h*0.5f, (f32)(-h)*0.5f);

    gpu_camera_set(gs->gpu, v3(0, 0, 0), 0);
    
    // Game present
    gpu_render_target_draw(gs->gpu, 0, 0, r2_width(game_view), r2_height(game_view), 0, gs->render_target);
    
    // UI draw
    ui_render(gs->gpu, &gs->ui);

    if(gs->paused) {
        char *text = "Pause";
        R2 dim = font_size_text(am_get_font(gs->am, "times.ttf", 64), text);
        f32 pos_x = 0 - (f32)r2_width(dim)*0.5f;
        f32 pos_y = 0 - (f32)r2_height(dim)*0.5f + VIRTUAL_RES_Y*0.25f;
        font_draw_text(gs->gpu, am_get_font(gs->am, "times.ttf", 64), text, pos_x, pos_y, v4(1, 1, 1, 1));
    }

    if(gs->debug_show) {
        cs_render(gs->gpu, &gs->cs);
        av_render(gs->gpu, &gs->av);

        static char text[1024];
        snprintf(text, 1024, "FPS: %d | MS %.2f", gs->FPS, gs->MS);
        R2 fps_dim = font_size_text(am_get_font(gs->am, "LiberationMono-Regular.ttf", 48), text);
        f32 pos_x = DEBUG_PADDING_X + -r2_width(display)*0.5f;
        f32 pos_y = -DEBUG_PADDING_Y + r2_height(display)*0.5f - (f32)r2_height(fps_dim);
        font_draw_text(gs->gpu, am_get_font(gs->am, "LiberationMono-Regular.ttf", 48), text, pos_x, pos_y, v4(1, 1, 1, 1));
    }
    gpu_render_target_end(gs->gpu, 0);

    gpu_frame_end(gs->gpu);
}

void game_shutdown(Memory *memory) {
    GameState *gs = game_state(memory);
    gpu_unload(gs->gpu);
}

void game_resize(Memory *memory, u32 w, u32 h) {
    GameState *gs = game_state(memory);
    unused(gs);

    display = r2_from_wh(0, 0, (i32)w, (i32)h);

    f32 vw = VIRTUAL_RES_X;
    f32 vh = VIRTUAL_RES_Y;
    f32 dw = (f32)r2_width(display);
    f32 dh = (f32)r2_height(display);
    f32 wvr = vw/vh;
    f32 hvr = vh/vw;
    
    vw = dw;
    vh = dw * hvr;
    
    if(vh > dh) {
        vw = dh * wvr;
        vh = dh;
    }

#if 0
    if(vw > dw) {
        vh = (i32)((f32)w * hvr);
        vw = (i32)((f32)h * wvr);
    }
#endif

    f32 x = dw*0.5f - vw*0.5f;
    f32 y = dh*0.5f - vh*0.5f;

    game_view = r2_from_wh((i32)x, (i32)y, (i32)vw, (i32)vh);

}
