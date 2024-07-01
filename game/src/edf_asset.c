//
// Created by tomas on 1/7/2024.
//

#include "edf_asset.h"

AssetManager *am_load(Arena *arena, Gpu gpu) {
    AssetManager *am = (AssetManager *)arena_push(arena, sizeof(*am), 8);
    am->arena = arena;
    am->gpu = gpu;
    for(u32 i = 0; i < array_len(am->assets_table); ++i) {
        am->assets_table[i].header.hash = ASSET_TABLE_INVALID_HASH;
    }
    return am;
}

Asset *am_get_asset(AssetManager *am, u32 hash) {
    assert(am->assets_table_used <= (u32)(MAX_ASSET_TABLE_SIZE*0.7f));

    u32 index = hash % MAX_ASSET_TABLE_SIZE;

    u32 start_index = index;
    Asset *asset = am->assets_table + index;
    while((asset->header.hash != ASSET_TABLE_INVALID_HASH) && (asset->header.hash != hash)) {
        index = (index + 1) % MAX_ASSET_TABLE_SIZE;
        asset = am->assets_table + index;
        assert(index != start_index);
    }

    return  asset;
}

Texture am_get_texture(AssetManager *am, char *path) {
    u32 hash = djb2(path);
    Asset *asset = am_get_asset(am, hash);

    if(asset->header.hash == hash) {
        assert(asset->header.type == ASSET_TYPE_TEXTURE);
        return asset->texture.texutre;
    } else {
        assert(asset->header.hash == ASSET_TABLE_INVALID_HASH);
        asset->header.type = ASSET_TYPE_TEXTURE;
        asset->header.hash = hash;
        asset->texture.bitmap = bitmap_load(am->arena, path);
        asset->texture.texutre = gpu_texture_load(am->gpu, &asset->texture.bitmap);
        am->assets_table_used++;
        return  asset->texture.texutre;
    }
}

static inline b32 find_font_info(AssetManager *am, char *path, FontInfo *font_info) {
    for(u32 i = 0; i < am->loaded_font_count; ++i) {
        FontInfo font_info_to_test = am->loaded_font_cache[i];
        if(strcmp(path, font_info_to_test.name) == 0) {
            *font_info = font_info_to_test;
            return true;
        }
    }
    return false;
}

Font *am_get_font(AssetManager *am, char *path, u32 size) {

    u32 hash = djb2(path) + (size << 3);
    Asset *asset = am_get_asset(am, hash);

    if(asset->header.hash == hash) {
        assert(asset->header.type == ASSET_TYPE_FONT);
        return asset->font.font;
    } else {
        FontInfo font_info;
        b32 found = find_font_info(am, path, &font_info);
        if(!found) {
            font_info = font_info_load(am->arena, path);
            if(am->loaded_font_count++ < array_len(am->loaded_font_cache)) {
                am->loaded_font_cache[am->loaded_font_count++] = font_info;
            }
        }

        assert(asset->header.hash == ASSET_TABLE_INVALID_HASH);
        asset->header.type = ASSET_TYPE_FONT;
        asset->header.hash = hash;
        asset->font.font = font_load(am->gpu, am->arena, font_info.info, (f32)size);
        am->assets_table_used++;
        return  asset->font.font;
    }
}
