#include "edf.h"
#include "edf_memory.h"
#include "edf_math.h"

void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);
    
    gs->platform_arena = arena_create(memory, mb(20));
    gs->gpu = gpu_load(&gs->platform_arena);
    gs->angle = 0;
}

void game_update(Memory *memory, Input *input, f32 dt) {
    GameState *gs = game_state(memory);

    gs->angle += dt;
}

void game_render(Memory *memory) {
    GameState *gs = game_state(memory);
    
    gpu_frame_begin(gs->gpu);
    
    gpu_draw_quad_color(gs->gpu, 0, 0, 100, 200, 0, v3(0, 1, 0));

    gpu_draw_quad_color(gs->gpu, 200, 400, 100, 200, gs->angle, v3(1, 0, 0));

    gpu_frame_end(gs->gpu);
}

void game_shutdown(Memory *memory) {
    GameState *gs = game_state(memory);

    gpu_unload(gs->gpu);
}

void game_resize(struct Memory *memory, u32 w, u32 h) {
    GameState *gs = game_state(memory);
    gpu_resize(gs->gpu, w, h);
}

