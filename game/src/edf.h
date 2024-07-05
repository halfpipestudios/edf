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

#if 0
#define VIRTUAL_RES_X 1920
#define VIRTUAL_RES_Y 1080
#define VIRTUAL_RATIO ((f32)VIRTUAL_RES_Y / (f32)VIRTUAL_RES_X)

#define MAP_COORDS_X 10.0f
#define MAP_COORDS_Y MAP_COORDS_X*VIRTUAL_RATIO

extern R2 display;
extern R2 game_view;
#endif

#define VIRTUAL_RATIO ((f32)9/(f32)16)
#define MAP_COORDS_X 10.0f
#define MAP_COORDS_Y MAP_COORDS_X*VIRTUAL_RATIO

typedef struct Display {
    R2 screen;
    R2 game_view;
    u32 resolution_index;
} Display;

#define VIRTUAL_RES_X_INDEX 0
#define VIRTUAL_RES_Y_INDEX 1
#define RESOLUTION_COUNT 6
extern u32 resolutions[RESOLUTION_COUNT][2];
extern Display display;
u32 virtual_w(void);
u32 virtual_h(void);

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
    Button *res_button;
    Button *frame_buffer_button;

    b32 paused;
    b32 debug_show;
    b32 show_frame_buffer;
    
    struct AssetManager *am;
    struct EntityManager *em;
    struct Entity *hero;

    struct Level *level;
    Texture ship_texture[2];
    Texture explotion_textures[11];
    Texture confeti_texture[5];
    Animation *explotion_anim;

    struct ParticleSystem *fire;
    struct ParticleSystem *confeti;
    struct ParticleSystem *neon;
    struct ParticleSystem *smoke;
    struct ParticleSystem *pixel;
    struct ParticleSystem *ps;
 
    f32 time_per_frame;
    i32 fps_counter;
    i32 FPS;
    f32 MS;
    Console cs;
    ArenaViewer av;

#define MAX_STARS 100
#define MAX_GALAXY 6
    Sprite stars[MAX_STARS];
    Sprite galaxy[MAX_GALAXY];
    
    RenderTarget render_targets[RESOLUTION_COUNT];
} GameState;

#define game_state(memory) ((GameState *)(memory)->data);
#define game_state_init(memory) ((memory)->used = (memory)->used + sizeof(GameState))

void game_init(struct Memory *memory);
void game_update(struct Memory *memory, Input *input, f32 dt);
void game_render(struct Memory *memory);
void game_shutdown(struct Memory *memory);
void game_resize(struct Memory *memory, u32 w, u32 h);

#endif // EDF_H
