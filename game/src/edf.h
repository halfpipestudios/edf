#ifndef EDF_H
#define EDF_H

#include "edf_font.h"
#include "edf_memory.h"
#include "edf_platform.h"

#define GAME_MEMORY_SIZE mb(256)
struct Memory;

typedef struct GameState {
    Gpu gpu;
    Spu spu;

    Arena platform_arena;
    Arena game_arena;

    Font *arial;
    Font *times;

    Bitmap ship_bitmap;
    Bitmap move_outer_bitmap;
    Bitmap move_inner_bitmap;
    Bitmap boost_bitmap;

    Texture ship_texture;
    Texture move_outer_texture;
    Texture move_inner_texture;
    Texture boost_texture;

    V3 boost_tint;

    Sprite *ship;
    V2 ship_vel;
    V2 ship_acc;
    f32 ship_damping;

    f32 s_inner;

    i32 joystick_location;
    bool joystick_is_down;
    f32 joystick_max_distance;
    f32 joystick_scale;
    V2 s_pos, c_pos;

    i32 button_location;
    bool button_is_down[2];
    V2 button_center;
    f32 button_radii;

} GameState;

#define game_state(memory) ((GameState *)(memory)->data);
#define game_state_init(memory) ((memory)->used = (memory)->used + sizeof(GameState))

void game_init(struct Memory *memory);
void game_update(struct Memory *memory, Input *input, f32 dt);
void game_render(struct Memory *memory);
void game_shutdown(struct Memory *memory);
void game_resize(struct Memory *memory, u32 w, u32 h);

#endif // EDF_H
