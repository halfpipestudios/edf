#ifndef EDF_H
#define EDF_H

#include "edf_font.h"
#include "edf_debug.h"
#include "edf_memory.h"
#include "edf_platform.h"
#include "edf_input.h"
#include "edf_collision.h"

#define GAME_MEMORY_SIZE mb(256)
struct Memory;
struct Entity;
struct EntityManager;
struct ParticleSystem;
struct Level;

#define MAX_STARS 1000
#define MAX_GALAXY 6

typedef struct GameState {

    Gpu gpu;
    Spu spu;
    Multitouch mt;
    Ui ui;

    Arena platform_arena;
    Arena game_arena;

    Joystick *joystick;
    Button *boost_button;
    Button *pause_button;
    Button *next_ship_button;
    Button *next_boost_button;
    Button *debug_button;

    b32 paused;
    b32 debug_show;

    struct EntityManager *em;
    struct Entity *hero;

    struct Level *level;

    struct ParticleSystem *fire;
    struct ParticleSystem *confeti;
    struct ParticleSystem *neon;
    struct ParticleSystem *smoke;
    struct ParticleSystem *pixel;
    struct ParticleSystem *ps;

    struct Animation *explotion_anim;
 
    f32 time_per_frame;
    i32 fps_counter;
    i32 FPS;
    Console cs;

    Font *liberation;
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
    Bitmap pause_bitmap;
    Bitmap rocks_bitmap;
    Bitmap rocks_full_bitmap;
    Bitmap rocks_corner_bitmap;
    Bitmap explotion_bitmaps[11];
    Bitmap square_bitmap;
    
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
    Texture pause_texture;
    Texture rocks_texutre;
    Texture rocks_full_texture;
    Texture rocks_corner_texture;
    Texture explotion_textures[11];
    Texture square_texture;
    

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
