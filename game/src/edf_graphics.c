#include "edf_memory.h"
#include "edf_platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Bitmap bitmap_load(Arena *arena, char *path) {
    File file     = os_file_read(arena, path);
    Bitmap bitmap = { 0 };
    i32 width, height, channels;
    u8 *data = stbi_load_from_memory(file.data, (i32)file.size, &width, &height, &channels, 4);
    if(!data) {
        return bitmap;
    }
    bitmap.data = (u32 *)data;
    bitmap.w    = width;
    bitmap.h    = height;
    return bitmap;
}

Bitmap bitmap_empty(Arena *arena, i32 w, i32 h, sz pixel_size) {
    Bitmap result;
    result.w    = w;
    result.h    = h;
    result.data = arena_push(arena, w * h * pixel_size, 8);
    return result;
}

void bitmap_copy_u8_u32(struct Arena *arena, Bitmap *bitmap8, Bitmap *bitmap32) {
    for(i32 y = 0; y < bitmap8->h; ++y) {
        for(i32 x = 0; x < bitmap8->w; ++x) {
            u8 src = ((u8 *)bitmap8->data)[y * bitmap8->w + x];
            ((u32 *)bitmap32->data)[y * bitmap32->w + x] =
                (src << 24) | (src << 16) | (src << 8) | src;
        }
    }
}

Bitmap bitmap_copy(struct Arena *arena, Bitmap *bitmap, sz pixel_size) {
    Bitmap result = bitmap_empty(arena, (i32)bitmap->w, (i32)bitmap->h, pixel_size);
    memcpy(result.data, bitmap->data, bitmap->w * bitmap->h * pixel_size);
    return result;
}

Sprite *sprite_load(struct Arena *arena, V2 pos, V2 scale, V4 tint, f32 anlge, Texture texture) {
    Sprite *sprite  = (Sprite *)arena_push(arena, sizeof(Sprite), 8);
    sprite->pos     = pos;
    sprite->scale   = scale;
    sprite->tint    = tint;
    sprite->angle   = anlge;
    sprite->texture = texture;
    return sprite;
}

void sprite_draw(Gpu gpu, Sprite *sprite) {
    gpu_draw_quad_texture_tinted(gpu, sprite->pos.x, sprite->pos.y, sprite->scale.x,
                                 sprite->scale.y, sprite->angle, sprite->texture, sprite->tint);
}
