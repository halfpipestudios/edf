#ifndef EDF_H
#define EDF_H

#include "edf_font.h"
#include "edf_memory.h"
#include "edf_platform.h"

#define GAME_MEMORY_SIZE mb(256)
struct Memory;

typedef struct GameState {
    Gpu gpu;
    Arena platform_arena;

    Font *arial;
    Font *times;

    f32 angle;

    Bitmap bitmap;
    Texture texture;

    Bitmap bitmap1;
    Texture texture1;

    Bitmap orbe_bitmap;
    Texture orbe_texture;

    Bitmap laser_bitmap;
    Texture laser_texture;

} GameState;

#define game_state(memory) ((GameState *)(memory)->data);
#define game_state_init(memory)                                                \
  ((memory)->used = (memory)->used + sizeof(GameState))

void game_init(struct Memory *memory);
void game_update(struct Memory *memory, f32 dt);
void game_render(struct Memory *memory);
void game_shutdown(struct Memory *memory);
void game_resize(struct Memory *memory, u32 w, u32 h);

void game_touches_down(struct Memory *memory, struct Input *input);
void game_touches_up(struct Memory *memory, struct Input *input);
void game_touches_move(struct Memory *memory, struct Input *input);

#endif // EDF_H
