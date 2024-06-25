#include "edf.h"

#include <stdlib.h> 
#include <time.h>

i32 rand_range(i32 min, i32 max) {
    return (rand() % (max - min + 1)) + min;
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
    gs->ship_bitmap[1]    = bitmap_load(&gs->game_arena, "OG Eng.png");
    gs->move_outer_bitmap = bitmap_load(&gs->game_arena, "move_outer.png");
    gs->move_inner_bitmap = bitmap_load(&gs->game_arena, "move_inner.png");
    gs->boost_bitmap      = bitmap_load(&gs->game_arena, "boost.png");
    gs->star_bitmap       = bitmap_load(&gs->game_arena, "star.png");
    gs->galaxy_bitmap     = bitmap_load(&gs->game_arena, "stb_image.png");
    gs->planet1_bitmap    = bitmap_load(&gs->game_arena, "planet1.png");
    gs->planet2_bitmap    = bitmap_load(&gs->game_arena, "planet2.png");
    gs->satelite_bitmap   = bitmap_load(&gs->game_arena, "Satelite.png");
    gs->meteorito_bitmap   = bitmap_load(&gs->game_arena, "Meteorito.png");

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

    f32 size = 32.0f * 3.0f;
    srand(time(0));
    u32 ship_rand_texture = rand_range(0, 1);
    gs->ship = sprite_load(&gs->game_arena, v2(0, 0), v2(size, size), v3(1, 1, 1), 0, gs->ship_texture[ship_rand_texture]);
    gs->ship_vel = v2(0, 0);
    gs->ship_acc = v2(0, 0);
    gs->ship_damping = 0.4f;

    gs->s_pos = v2(-740, -250);
    gs->c_pos = gs->s_pos;
    gs->joystick_scale   = 4;
    
    gs->s_inner      = 1.5f;
    gs->button_radii = (gs->boost_bitmap.w * gs->joystick_scale * gs->s_inner) * 0.5f;

    gs->button_center  = v2(740, -250);
    gs->button_radii   = 100;

    gs->boost_tint = v3(1, 1, 1);

    gs->joystick_touch = -1;
    gs->button_touch = -1;

    gs->stars_init = false;

}

void game_update(Memory *memory, Input *input, f32 dt) {
    GameState *gs             = game_state(memory);
    
    if(gs->stars_init == false) {

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
            gs->meteorito_texture
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
        gs->stars_init = true;
    }



    gs->joystick_max_distance = ((f32)gs->move_outer_bitmap.w * 0.5f) * gs->joystick_scale;

#if 0
    os_print("[%d]: %d\n", 0, input->locations[0]);
    os_print("[%d]: %d\n", 1, input->locations[1]);
    os_print("[%d]: %d\n", 2, input->locations[2]);
    os_print("[%d]: %d\n", 3, input->locations[3]);
    os_print("[%d]: %d\n", 4, input->locations[4]);
#endif

    mt_begin(&gs->mt, input);

    mt_touch_just_in_circle(&gs->mt, &gs->button_touch, gs->button_center, gs->button_radii);
    
    i32 hw = os_display_width() * 0.5f;
    i32 hh = os_display_height() * 0.5f;

    R2 window_rect;
    window_rect.min.x = -hw;
    window_rect.max.x = 0;
    window_rect.min.y = -hh;
    window_rect.max.y = hh;

    if(mt_touch_in_rect(&gs->mt, &gs->joystick_touch, window_rect)) {
        gs->s_pos = mt_touch_pos(&gs->mt, gs->joystick_touch);
        gs->c_pos   = gs->s_pos;
    }

    V2 diff = v2_sub(gs->s_pos, gs->c_pos);
    f32 len = v2_len(diff);
    if(len > gs->joystick_max_distance) {
        V2 dir      = v2_normalized(diff);
        gs->s_pos.x = gs->c_pos.x + dir.x * gs->joystick_max_distance;
        gs->s_pos.y = gs->c_pos.y + dir.y * gs->joystick_max_distance;
    }


    if(mt_touch_down(&gs->mt, gs->joystick_touch)) {
        V2 pos = mt_touch_pos(&gs->mt, gs->joystick_touch);
        gs->c_pos = pos;
        if(len > (gs->joystick_max_distance * 0.2f)) {
            V2 dir          = v2_normalized(diff);
            gs->ship->angle = atan2f(dir.y, dir.x) + (PI / 2.0f);
        }
    } else {
        gs->c_pos = gs->s_pos;
    }

    gs->boost_tint = v3(1, 1, 1);
    if(mt_touch_down(&gs->mt, gs->button_touch)) {
        gs->boost_tint = v3(0.5f, 0.5f, 0.5f);
        V2 dir = {0};
        dir.x = cosf(gs->ship->angle + (PI / 2.0f));
        dir.y = sinf(gs->ship->angle + (PI / 2.0f));
        if(v2_len(dir) > 0) {
            dir = v2_normalized(dir);
            gs->ship_acc.x += dir.x * 800.0f;
            gs->ship_acc.y += dir.y * 800.0f;
        }

    }

    gs->ship->pos.x += gs->ship_vel.x * dt;
    gs->ship->pos.y += gs->ship_vel.y * dt;

    gs->ship_vel.x += gs->ship_acc.x * dt;
    gs->ship_vel.y += gs->ship_acc.y * dt;

    gs->ship_vel.x *= powf(gs->ship_damping, dt);
    gs->ship_vel.y *= powf(gs->ship_damping, dt);

    gs->ship_acc = v2(0, 0);

    R2 bounds = {0};
    bounds.min.x = gs->ship->pos.x - (hw * 1.25f);
    bounds.min.y = gs->ship->pos.y - (hh * 1.25f);
    bounds.max.x = gs->ship->pos.x + (hw * 1.25f);
    bounds.max.y = gs->ship->pos.y + (hh * 1.25f);


    for(i32 i = 0; i < MAX_GALAXY; i++) {
        Sprite *galaxy = gs->galaxy + i;
        galaxy->pos.x -= (gs->ship_vel.x / galaxy->z) * dt;
        galaxy->pos.y -= (gs->ship_vel.y / galaxy->z) * dt;
        
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
        star->pos.x -= (gs->ship_vel.x / star->z) * dt;
        star->pos.y -= (gs->ship_vel.y / star->z) * dt;
        
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


    mt_end(&gs->mt, input);
}

void game_render(Memory *memory) {
    GameState *gs = game_state(memory);

    u32 w = os_display_width();
    u32 h = os_display_height();

    gpu_frame_begin(gs->gpu);

    gpu_camera_set(gs->gpu, v3(0, 0, 0), 0);

    gpu_draw_quad_color(gs->gpu, 0, 0, w, h, 0, v3(0.05f, 0.05f, 0.1f));

    // Entities draw
    gpu_camera_set(gs->gpu, v3(gs->ship->pos.x, gs->ship->pos.y, 0), 0);

    gpu_blend_state_set(gs->gpu, GPU_BLEND_STATE_ALPHA);
    // draw the stars
    for(i32 i = 0; i < MAX_GALAXY; i++) {
        Sprite *galaxy = gs->galaxy + i;
        sprite_draw(gs->gpu, galaxy);
    }

    // draw the stars
    for(i32 i = 0; i < MAX_STARS; i++) {
        Sprite *star = gs->stars + i;
        sprite_draw(gs->gpu, star);
    }
    gpu_blend_state_set(gs->gpu, GPU_BLEND_STATE_ALPHA);
    
    sprite_draw(gs->gpu, gs->ship);

    // UI draw
    gpu_camera_set(gs->gpu, v3(0, 0, 0), 0);

    
    f32 s       = gs->joystick_scale;
    f32 s_inner = gs->s_inner;

    gpu_draw_quad_texture(gs->gpu, gs->s_pos.x, gs->s_pos.y, gs->move_outer_bitmap.w * s,
                          gs->move_outer_bitmap.h * s, 0, gs->move_outer_texture);
    gpu_draw_quad_texture(gs->gpu, gs->c_pos.x, gs->c_pos.y, gs->move_inner_bitmap.w * s * s_inner,
                          gs->move_inner_bitmap.h * s * s_inner, 0, gs->move_inner_texture);
    gpu_draw_quad_texture_tinted(gs->gpu, gs->button_center.x, gs->button_center.y, gs->button_radii * 2,
                          gs->button_radii * 2, 0, gs->boost_texture, gs->boost_tint);
    
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
