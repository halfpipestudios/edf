#include "edf.h"
#include "edf_memory.h"
#include "edf_math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

V3 v3(f32 x, f32 y, f32 z) {
    return (V3){ x, y, z };
}

Bitmap bitmap_load(struct Arena *arena, char *path) {
    File file = os_file_read(arena, path);
    Bitmap bitmap = {0};
    i32 width, height, channels;
    u8 *data = stbi_load_from_memory(file.data, file.size, &width, &height, &channels, 4);
    if(!data) {
        return bitmap;
    }
    bitmap.data = data;
    bitmap.width = width;
    bitmap.height = height;
    return bitmap; 
}

void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);
    
    gs->platform_arena = arena_create(memory, mb(20));

    gs->gpu = gpu_load(&gs->platform_arena);

    gs->bitmap = bitmap_load(&gs->platform_arena, "Player.png");

    gs->texture = gpu_texture_load(gs->gpu, &gs->bitmap);
    
    i32 StopHere = 0;
}

void game_update(Memory *memory, Input *input, f32 dt) {
    GameState *gs = game_state(memory);

}

void game_render(Memory *memory) {
    GameState *gs = game_state(memory);
    
    gpu_frame_begin(gs->gpu);
    
    gpu_draw_quad_texture(gs->gpu, 0, 0, gs->bitmap.width*1.5f, gs->bitmap.height*1.5f, 0, 0);

    gpu_frame_end(gs->gpu);
}

void game_shutdown(Memory *memory) {
    GameState *gs = game_state(memory);

    gpu_unload(gs->gpu);
}
