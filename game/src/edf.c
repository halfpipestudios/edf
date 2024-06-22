#include "edf.h"
#include "edf_math.h"
#include "edf_memory.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


Bitmap bitmap_load(Arena *arena, char *path) {
    File file = os_file_read(arena, path);
    Bitmap bitmap = {0};
    i32 width, height, channels;
    u8 *data = stbi_load_from_memory(file.data, file.size, &width, &height,
                                   &channels, 4);
    if (!data) {
        return bitmap;
    }
    bitmap.data = (u32 *)data;
    bitmap.width = width;
    bitmap.height = height;
    return bitmap;
}

Bitmap bitmap_empty(Arena *arena, i32 w, i32 h, sz pixel_size) {
    Bitmap result;
    result.width = w;
    result.height = h;
    result.data = arena_push(arena, w*h*pixel_size, 8);
    return result;
}

void bitmap_copy_u8_u32(struct Arena *arena, Bitmap *bitmap8, Bitmap *bitmap32) {

    for(u32 y = 0; y < bitmap8->height; ++y) {
        for(u32 x = 0; x < bitmap8->width; ++x) {
            u8 src = ((u8 *)bitmap8->data)[y*bitmap8->width+x];
            ((u32 *)bitmap32->data)[y*bitmap32->width+x] = (src << 24) | (src << 16) | (src << 8) | src;
        }
    }
}

Bitmap bitmap_copy(struct Arena *arena, Bitmap *bitmap, sz pixel_size) {
    Bitmap result = bitmap_empty(arena, (i32)bitmap->width, (i32)bitmap->height, pixel_size);
    memcpy(result.data, bitmap->data, bitmap->width*bitmap->height*pixel_size);
    return result;
}

void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);

    gs->platform_arena = arena_create(memory, mb(20));
    gs->gpu = gpu_load(&gs->platform_arena);

    gs->font = font_load(gs->gpu, &gs->platform_arena, "arial.ttf");

    gs->angle = 0;

    gs->bitmap = bitmap_load(&gs->platform_arena, "Player.png");
    gs->bitmap1 = bitmap_load(&gs->platform_arena, "link.png");
    gs->texture = gpu_texture_load(gs->gpu, &gs->bitmap);
    gs->texture1 = gpu_texture_load(gs->gpu, &gs->bitmap1);

    gs->orbe_bitmap = bitmap_load(&gs->platform_arena, "orbe.png");
    gs->orbe_texture = gpu_texture_load(gs->gpu, &gs->orbe_bitmap);

    gs->laser_bitmap = bitmap_load(&gs->platform_arena, "laser.png");
    gs->laser_texture = gpu_texture_load(gs->gpu, &gs->laser_bitmap);

}

void game_update(Memory *memory, f32 dt) {
    GameState *gs = game_state(memory);
    gs->angle += dt;
    if(gs->angle == 2*3.14) {
        gs->angle = 0;
    }
}

void game_render(Memory *memory) {
    GameState *gs = game_state(memory);

    u32 w = os_display_width();
    u32 h = os_display_height();

    gpu_frame_begin(gs->gpu);

    gpu_draw_quad_color(gs->gpu, 0, 0, w, h, 0, v3(0.1f, 0.1f, 0.15f));

    font_draw_text(gs->gpu, gs->font, "Hello, Sailor!", -200, -400, v3(1, 1, 1));

    f32 scale = 10;
    gpu_draw_quad_texture(gs->gpu, 0, 0, gs->bitmap.width * scale,
                        gs->bitmap.height * scale, gs->angle, gs->texture);

    gpu_draw_quad_texture(gs->gpu, 0, -600, gs->bitmap1.width * scale,
                        gs->bitmap1.height * scale, gs->angle, gs->texture1);

    gpu_blend_state_set(gs->gpu, GPU_BLEND_STATE_ADDITIVE);


    gpu_draw_quad_texture(gs->gpu, -100, 500, gs->orbe_bitmap.width * scale,
                        gs->orbe_bitmap.height * scale, 0, gs->orbe_texture);
    gpu_draw_quad_texture(gs->gpu, 0, 600, gs->orbe_bitmap.width * scale,
                        gs->orbe_bitmap.height * scale, 0, gs->orbe_texture);
    gpu_draw_quad_texture(gs->gpu, 0, 450, gs->orbe_bitmap.width * scale,
                        gs->orbe_bitmap.height * scale, 0, gs->orbe_texture);
    gpu_draw_quad_texture(gs->gpu, 100, 500, gs->orbe_bitmap.width * scale,
                        gs->orbe_bitmap.height * scale, 0, gs->orbe_texture);

    gpu_draw_quad_texture(gs->gpu, 0, 700, gs->laser_bitmap.width * scale,
                        gs->laser_bitmap.height * scale, 3.14/2.0, gs->laser_texture);


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

void game_touches_down(Memory *memory, Input *input) {
}

void game_touches_up(Memory *memory, Input *input) {
}

void game_touches_move(Memory *memory, Input *input) {
}
