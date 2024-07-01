//
// Created by tomas on 1/7/2024.
//

#ifndef EDF_ASSET_H
#define EDF_ASSET_H

#include "edf_common.h"
#include "edf_memory.h"
#include "edf_font.h"
#include "edf_platform.h"

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

    #define MAX_LOADED_FONT_CACHE 10
    FontInfo loaded_font_cache[MAX_LOADED_FONT_CACHE];
    u32 loaded_font_count;

} AssetManager;

AssetManager *am_load(Arena *arena, Gpu gpu);
Texture am_get_texture(AssetManager *am, char *path);
Font *am_get_font(AssetManager *am, char *path, u32 size);

#endif //EDF_ASSET_H
