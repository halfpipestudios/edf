#include "edf.h"

void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);

    init_scratch_arenas(memory, 3, mb(10));

    gs->platform_arena = arena_create(memory, mb(100));
    gs->gpu            = gpu_load(&gs->platform_arena);
    gs->spu            = spu_load(&gs->platform_arena);

    gs->arial = font_load(gs->gpu, &gs->platform_arena, "arial.ttf", 128);
    gs->times = font_load(gs->gpu, &gs->platform_arena, "times.ttf", 64);

    gs->angle = 0;

    gs->bitmap   = bitmap_load(&gs->platform_arena, "Player.png");
    gs->bitmap1  = bitmap_load(&gs->platform_arena, "link.png");
    gs->texture  = gpu_texture_load(gs->gpu, &gs->bitmap);
    gs->texture1 = gpu_texture_load(gs->gpu, &gs->bitmap1);

    gs->orbe_bitmap  = bitmap_load(&gs->platform_arena, "orbe.png");
    gs->orbe_texture = gpu_texture_load(gs->gpu, &gs->orbe_bitmap);

    gs->laser_bitmap  = bitmap_load(&gs->platform_arena, "laser.png");
    gs->laser_texture = gpu_texture_load(gs->gpu, &gs->laser_bitmap);

    gs->test_wave  = wave_load(&gs->platform_arena, "test.wav");
    gs->test_sound = spu_sound_add(gs->spu, &gs->test_wave, false, true);
}

void game_update(Memory *memory, f32 dt) {
    GameState *gs = game_state(memory);
    gs->angle += dt;
    if(gs->angle == 2 * 3.14) {
        gs->angle = 0;
    }
}

void game_render(Memory *memory) {
    GameState *gs = game_state(memory);

    u32 w = os_display_width();
    u32 h = os_display_height();

    gpu_frame_begin(gs->gpu);

    gpu_draw_quad_color(gs->gpu, 0, 0, w, h, 0, v3(0.1f, 0.1f, 0.15f));

    {
        char *text   = "Tomas Cabrerizo";
        R2 text_size = font_size_text(gs->arial, text, -400, 200);
        f32 pos_x    = (f32)text_size.min.x + (f32)r2_width(text_size) * 0.5f;
        f32 pos_y    = (f32)text_size.min.y + (f32)r2_height(text_size) * 0.5f;
        gpu_draw_quad_color(gs->gpu, pos_x, pos_y, (f32)r2_width(text_size),
                            (f32)r2_height(text_size), 0, v3(0, 1, 0));
        font_draw_text(gs->gpu, gs->arial, text, -400, 200, v3(0, 0, 1));
    }

    {
        char *text   = "Hello, Sailor!";
        R2 text_size = font_size_text(gs->times, text, -200, -400);
        f32 pos_x    = (f32)text_size.min.x + (f32)r2_width(text_size) * 0.5f;
        f32 pos_y    = (f32)text_size.min.y + (f32)r2_height(text_size) * 0.5f;
        gpu_draw_quad_color(gs->gpu, pos_x, pos_y, (f32)r2_width(text_size),
                            (f32)r2_height(text_size), 0, v3(0, 0, 1));
        font_draw_text(gs->gpu, gs->times, text, -200, -400, v3(1, 0, 1));
    }

    font_draw_text(gs->gpu, gs->arial, "Jose Lagos", -200, -1000, v3(1, 0.7f, 0.2f));

    f32 scale = 10;
    gpu_draw_quad_texture(gs->gpu, 0, 0, gs->bitmap.w * scale, gs->bitmap.h * scale, gs->angle,
                          gs->texture);

    gpu_draw_quad_texture(gs->gpu, 0, -600, gs->bitmap1.w * scale, gs->bitmap1.h * scale, gs->angle,
                          gs->texture1);

    gpu_blend_state_set(gs->gpu, GPU_BLEND_STATE_ADDITIVE);
    gpu_draw_quad_texture(gs->gpu, -100, 500, gs->orbe_bitmap.w * scale, gs->orbe_bitmap.h * scale,
                          0, gs->orbe_texture);
    gpu_draw_quad_texture(gs->gpu, 0, 600, gs->orbe_bitmap.w * scale, gs->orbe_bitmap.h * scale, 0,
                          gs->orbe_texture);
    gpu_draw_quad_texture(gs->gpu, 0, 450, gs->orbe_bitmap.w * scale, gs->orbe_bitmap.h * scale, 0,
                          gs->orbe_texture);
    gpu_draw_quad_texture(gs->gpu, 100, 500, gs->orbe_bitmap.w * scale, gs->orbe_bitmap.h * scale,
                          0, gs->orbe_texture);

    gpu_draw_quad_texture(gs->gpu, 0, 700, gs->laser_bitmap.w * scale, gs->laser_bitmap.h * scale,
                          3.14f / 2.0f, gs->laser_texture);

    gpu_blend_state_set(gs->gpu, GPU_BLEND_STATE_ALPHA);
    gpu_draw_quad_color(gs->gpu, 200, 400, 100, 200, gs->angle, v3(1, 0, 0));

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

void game_touches_down(struct Memory *memory, struct Input *input) {
    unused(input);
    GameState *gs = game_state(memory);
    spu_sound_play(gs->spu, gs->test_sound);
    os_print("down\n");
}

void game_touches_up(struct Memory *memory, struct Input *input) {
    unused(input);
    GameState *gs = game_state(memory);
    spu_sound_pause(gs->spu, gs->test_sound);
    os_print("up\n");
}

void game_touches_move(struct Memory *memory, struct Input *input) {
    unused(memory);
    unused(input);
}
