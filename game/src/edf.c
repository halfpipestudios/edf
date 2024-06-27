#include "edf.h"
#include "edf_entity.h"
#include "sys/edf_render_sys.h"
#include "sys/edf_input_sys.h"
#include "sys/edf_physics_sys.h"
#include "edf_particles.h"

#include <stdarg.h>
#include <stdio.h>

void stars_init(GameState *gs) {
    static u32 star_colors[4] = {
        0xFFFDF4F5,
        0xFFE8A0BF,
        0xFFBA90C6,
        0xFFC0DBEA
    };

    i32 hw = (VIRTUAL_RES_X * 0.5f) * 1.25f;
    i32 hh = (VIRTUAL_RES_Y * 0.5f) * 1.25f;
    
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
        galaxy->tint = v4(0.4f, 0.4f, 0.4f, 1);
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
        star->tint = hex_to_v4(star_colors[color_index]);
        star->angle = 0;
        
        color_index = (color_index + 1) % array_len(star_colors);
    }
}

void stars_update(GameState *gs, f32 dt) {
    i32 hw = VIRTUAL_RES_X * 0.5f;
    i32 hh = VIRTUAL_RES_Y * 0.5f;

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

PARTICLE_SYSTEM_UPDATE(ship_ps_update) {

    static u32 fire_tint_index = 0;
    static u32 fire_tint[] = {
        0xffff4200,
        0xffffa700,
        0xfffff796
    };

    if(v2_len(particle->vel) == 0.0f) {
        f32 rand = ((f32)rand_range(-20, 20)/ 180.0f) * PI;
        f32 offset = (PI*0.5f) + rand;
        V2 dir;
        dir.x = -cosf(gs->hero->angle + offset);
        dir.y = -sinf(gs->hero->angle + offset);
        particle->vel.x = dir.x * 180.0f + gs->hero->vel.x;
        particle->vel.y = dir.y * 180.0f + gs->hero->vel.y;
        particle->tint = hex_to_v4(fire_tint[fire_tint_index]);
        fire_tint_index = (fire_tint_index + 1) % array_len(fire_tint);
    }
    
    float x = particle->lifetime;
    particle->scale =  clamp((1.0f-(2*x-1)*(2*x-1)) * 60, 5, 60);
    
    x = particle->lifetime/particle->save_lifetime;
    particle->tint.w = x*x;
    
    particle->pos.x += particle->vel.x * dt;
    particle->pos.y += particle->vel.y * dt;
}


PARTICLE_SYSTEM_UPDATE(confeti_ps_update) {
    static u32 confeti_tint_index = 0;
    static u32 confeti_tint[] = {
        0xffa864fd,
        0xff29cdff,
        0xff78ff44,
        0xffff718d,
        0xfffdff6a
    };

    if(v2_len(particle->vel) == 0.0f) {
        
        f32 rand = ((f32)rand_range(-20, 20)/ 180.0f) * PI;
        f32 offset = (PI*0.5f) + rand;
        V2 dir;
        dir.x = -cosf(gs->hero->angle + offset);
        dir.y = -sinf(gs->hero->angle + offset);
        V2 perp = v2(-dir.y, dir.x);

        f32 x_offet = rand_range(gs->hero->scale.x*-0.15f, gs->hero->scale.x*0.15f);
        particle->pos.x += perp.x * x_offet;
        particle->pos.y += perp.y * x_offet;
        
        particle->vel.x = dir.x * (f32)rand_range(100, 300) + gs->hero->vel.x;
        particle->vel.y = dir.y * (f32)rand_range(100, 300) + gs->hero->vel.y;
        particle->scale = 20.0f;
        particle->save_lifetime = (f32)rand_range(25, 400) / 100.0f;
        particle->lifetime = (f32)rand_range(25, 400) / 100.0f;
        particle->tex = gs->confeti_texture[confeti_tint_index];
        particle->tint = hex_to_v4(confeti_tint[confeti_tint_index]);
        confeti_tint_index = (confeti_tint_index + 1) % array_len(confeti_tint);
        particle->angular_vel = (f32)rand_range(-20, 20);
    }
    if((particle->lifetime / particle->save_lifetime) < 0.15) {
        particle->tint.w = particle->lifetime;
    }
    
    particle->angle += particle->angular_vel * dt;
    particle->pos.x += particle->vel.x * dt;
    particle->pos.y += particle->vel.y * dt;
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
    gs->orbe_bitmap   = bitmap_load(&gs->game_arena, "orbe.png");
    gs->confeti_bitmap[0] = bitmap_load(&gs->game_arena, "confeti1.png");
    gs->confeti_bitmap[1] = bitmap_load(&gs->game_arena, "confeti2.png");
    gs->confeti_bitmap[2] = bitmap_load(&gs->game_arena, "confeti3.png");
    gs->confeti_bitmap[3] = bitmap_load(&gs->game_arena, "confeti4.png");
    gs->confeti_bitmap[4] = bitmap_load(&gs->game_arena, "confeti5.png");
    gs->pause_bitmap      = bitmap_load(&gs->game_arena, "pause.png");

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
    gs->orbe_texture       = gpu_texture_load(gs->gpu, &gs->orbe_bitmap);
    gs->confeti_texture[0] = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[0]);
    gs->confeti_texture[1] = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[1]);
    gs->confeti_texture[2] = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[2]);
    gs->confeti_texture[3] = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[3]);
    gs->confeti_texture[4] = gpu_texture_load(gs->gpu, &gs->confeti_bitmap[4]);
    gs->pause_texture      = gpu_texture_load(gs->gpu, &gs->pause_bitmap);

    gs->em = entity_manager_load(&gs->game_arena, 100);

    // hero initialization
    f32 size = 32.0f * 3.0f;
    srand(time(0));
    
    u32 ps_rand = rand_range(0, 1);
    if(ps_rand == 1) {
        gs->ps = particle_system_create(&gs->game_arena,
                                        100, 10,
                                        0.05f, v2(0, 0), gs->orbe_texture,
                                        ship_ps_update, GPU_BLEND_STATE_ADDITIVE);
    } else {
        gs->ps = particle_system_create(&gs->game_arena,
                                        1000, 10,
                                        0.05f, v2(0, 0), 0,
                                        confeti_ps_update, GPU_BLEND_STATE_ALPHA);
        
    }
    
    
    u32 ship_rand_texture = rand_range(0, 1);
    gs->hero = entity_manager_add_entity(gs->em);
    entity_add_input_component(gs->hero);
    entity_add_render_component(gs->hero, v3(0, 0, 0), v2(size, size), gs->ship_texture[ship_rand_texture], v4(1, 1, 1, 1));
    entity_add_physics_component(gs->hero, v2(0, 0), v2(0, 0), 0.4f);

    i32 hw = VIRTUAL_RES_X * 0.5f;
    i32 hh = VIRTUAL_RES_Y * 0.5f;
    R2 window_rect;
    window_rect.min.x = -hw;
    window_rect.max.x = 0;
    window_rect.min.y = -hh;
    window_rect.max.y = hh;
    gs->joystick = ui_joystick_alloc(&gs->ui, &gs->game_arena, v2(-740, -250), window_rect, 
                                    140, 220, gs->move_inner_texture, gs->move_outer_texture);

    gs->boost_button = ui_button_alloc(&gs->ui, &gs->game_arena, v2(740, -250), 135, gs->boost_texture);

    f32 paddin_top = 20;
    f32 pause_buttom_dim = 100;
    V2 pause_button_pos = v2(0, VIRTUAL_RES_Y*0.5f-pause_buttom_dim*0.5f-paddin_top);
    gs->pause_button = ui_button_alloc(&gs->ui, &gs->game_arena, pause_button_pos, pause_buttom_dim*0.5f, gs->pause_texture);

    stars_init(gs);
}

void game_update(Memory *memory, Input *input, f32 dt) {
    
    GameState *gs = game_state(memory);

    ui_begin(&gs->ui, &gs->mt, input, dt);

    if(ui_button_just_up(&gs->mt, gs->pause_button)) {
        gs->paused = !gs->paused;
    }

    if(!gs->paused) {
        input_system_update(gs, gs->em, dt);
        physics_system_update(gs->em, dt);
        stars_update(gs, dt);

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

    gpu_camera_set(gs->gpu, v3(gs->hero->pos.x, gs->hero->pos.y, 0), 0);
    stars_render(gs);

    //gpu_blend_state_set(gs->gpu, GPU_BLEND_STATE_ADDITIVE);
    particle_system_render(gs->gpu, gs->ps);
    //gpu_blend_state_set(gs->gpu, GPU_BLEND_STATE_ALPHA);

    // Entities draw
    render_system_update(gs, gs->em);



    // UI draw
    gpu_camera_set(gs->gpu, v3(0, 0, 0), 0);
    ui_render(gs->gpu, &gs->ui);
    
    {
        static char fps_text[1024];
        snprintf(fps_text, 1024, "FPS: %d", gs->FPS);
        R2 dim = font_size_text(gs->times, fps_text);
        f32 pos_x = -VIRTUAL_RES_X*0.5f;
        f32 pos_y = VIRTUAL_RES_Y*0.5f - r2_height(dim);
        font_draw_text(gs->gpu, gs->times, fps_text, pos_x, pos_y, v4(1, 1, 1, 1));
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
