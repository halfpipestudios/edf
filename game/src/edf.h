#ifndef EDF_H
#define EDF_H

#include "edf_font.h"
#include "edf_memory.h"
#include "edf_platform.h"
#include "edf_input.h"

#define VIRTUAL_RES_X 1920
#define VIRTUAL_RES_Y 1080

#define GAME_MEMORY_SIZE mb(256)
struct Memory;
struct Entity;
struct EntityManager;
struct ParticleSystem;

#define MAX_STARS 75
#define MAX_GALAXY 6

typedef struct GameState {
    
    Gpu gpu;
    Spu spu;
    Multitouch mt;
    Ui ui;

    Arena platform_arena;
    Arena game_arena;

    Joystick *joystick;
    Joystick *joystick2;
    Button *button;
    Button *button2;

    struct Entity *hero;
    struct EntityManager *em;

    struct ParticleSystem *ps;
 
    f32 time_per_frame;
    i32 fps_counter;
    i32 FPS;
    V2 DEBUG_touch;

    Font *arial;
    Font *times;

    Bitmap ship_bitmap[2];
    Bitmap move_outer_bitmap;
    Bitmap move_inner_bitmap;
    Bitmap boost_bitmap;
    Bitmap star_bitmap;
    Bitmap galaxy_bitmap;
    Bitmap planet1_bitmap;
    Bitmap planet2_bitmap;
    Bitmap satelite_bitmap;
    Bitmap meteorito_bitmap;
    Bitmap deathstar_bitmap;
    Bitmap orbe_bitmap;
    Bitmap confeti_bitmap[5];

    Texture ship_texture[2];
    Texture move_outer_texture;
    Texture move_inner_texture;
    Texture boost_texture;
    Texture star_texture;
    Texture galaxy_texture;
    Texture planet1_texture;
    Texture planet2_texture;
    Texture satelite_texture;
    Texture meteorito_texture;
    Texture deathstar_texture;
    Texture orbe_texture;
    Texture confeti_texture[5];

    Sprite stars[MAX_STARS];
    Sprite galaxy[MAX_GALAXY];

} GameState;

#define game_state(memory) ((GameState *)(memory)->data);
#define game_state_init(memory) ((memory)->used = (memory)->used + sizeof(GameState))

void game_init(struct Memory *memory);
void game_update(struct Memory *memory, Input *input, f32 dt);
void game_render(struct Memory *memory);
void game_shutdown(struct Memory *memory);
void game_resize(struct Memory *memory, u32 w, u32 h);

#endif // EDF_H
