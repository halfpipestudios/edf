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

typedef enum AssetType {
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_SOUND,
    ASSET_TYPE_FONT,
} AssetType;

typedef struct AssetHeader {
    AssetType type;
    u32 hash;
} AssetHeader;

typedef struct AssetTexture {
    AssetHeader header;
    Texture texutre;
    Bitmap bitmap;
} AssetTexture;

typedef struct AssetSound {
    AssetHeader header;
    Sound sound;
    Wave wave;
} AssetSound;

typedef struct AssetFont {
    AssetHeader header;
    Font *font;
} AssetFont;

typedef union Asset {
    AssetHeader header;
    AssetTexture texture;
    AssetSound sound;
    AssetFont font;
} Asset;

static inline u32 djb2(char *str) {
    u32 hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

typedef struct AssetManager {
    Arena *arena;
    Gpu gpu;
    #define ASSET_TABLE_INVALID_HASH ((u32)-1)
    #define MAX_ASSET_TABLE_SIZE 1024
    Asset assets_table[MAX_ASSET_TABLE_SIZE];
    u32 assets_table_used;
} AssetManager;

AssetManager *am_load(Arena *arena, Gpu gpu);
Texture am_get_texture(AssetManager *am, char *path);
Font *am_get_font(AssetManager *am, char *path, u32 size);

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
    Console cs;

#define MAX_STARS 1000
#define MAX_GALAXY 6
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
