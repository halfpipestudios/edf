#ifndef EDF_GRAPHICS_H
#define EDF_GRAPHICS_H

#include "edf_common.h"
#include "edf_math.h"

typedef void *Gpu;
typedef void *Texture;

struct Arena;

typedef struct Bitmap {
    void *data;
    i32 w;
    i32 h;
} Bitmap;

Bitmap bitmap_load(struct Arena *arena, char *path);
Bitmap bitmap_empty(struct Arena *arena, i32 w, i32 h, sz pixel_size);
Bitmap bitmap_copy(struct Arena *arena, Bitmap *bitmap, sz pixel_size);
void bitmap_copy_u8_u32(struct Arena *arena, Bitmap *bitmap8, Bitmap *bitmap32);

typedef struct Sprite {
    Texture texture;
    V2 pos;
    V2 scale;
    V3 tint;
    f32 angle;
} Sprite;

Sprite *sprite_load(struct Arena *arena, V2 pos, V2 scale, V3 tint, f32 anlge, Texture texture);
void sprite_draw(Gpu gpu, Sprite *sprite);

#endif /* EDF_GRAPHICS_H */
