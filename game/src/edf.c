#include "edf.h"
#include "edf_memory.h"
#include "edf_math.h"


V3 v3(f32 x, f32 y, f32 z) {
    return (V3){ x, y, z };
}


void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);
    
    gs->platform_arena = arena_create(memory, mb(20));

    gs->gpu = gpu_load(&gs->platform_arena);

    File file = os_file_read(&gs->platform_arena, "link.png");
}

void game_update(Memory *memory, Input *input, f32 dt) {
    GameState *gs = game_state(memory);

}

void game_render(Memory *memory) {
    GameState *gs = game_state(memory);
    
    gpu_frame_begin(gs->gpu);
    
    gpu_draw_quad_color(gs->gpu, 0, 0, 100, 200, 0, v3(0, 1, 0));

    gpu_frame_end(gs->gpu);
}

void game_shutdown(Memory *memory) {
    GameState *gs = game_state(memory);

    gpu_unload(gs->gpu);
}
