#include "edf.h"
#include "edf_memory.h"


void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);
    
    gs->platform_arena = arena_create(memory, mb(20));

    gs->gpu = gpu_load(&gs->platform_arena);
}

void game_update(Memory *memory, Input *input, f32 dt) {
    GameState *gs = game_state(memory);

}

void game_render(Memory *memory) {
    GameState *gs = game_state(memory);
    
    gpu_frame_begin(gs->gpu);


    gpu_frame_end(gs->gpu);
}

void game_shutdown(Memory *memory) {
    GameState *gs = game_state(memory);

    gpu_unload(gs->gpu);
}
