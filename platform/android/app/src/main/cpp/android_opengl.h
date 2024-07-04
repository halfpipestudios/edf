//
// Created by tomas on 21/6/2024.
//

#ifndef EDF_ANDROID_OPENGL_H
#define EDF_ANDROID_OPENGL_H

#include <GLES3/gl3.h>
#include <edf.h>

typedef struct OpenglVertex {
    V2 pos;
    V2 uvs;
    V4 color;
} OpenglVertex;

static V3 vertices[] = {
    {  0.5f,  -0.5f, 0 }, // bottom right
    { -0.5f,  -0.5f, 0 }, // bottom left
    { -0.5f,   0.5f, 0 }, // top left
    {  0.5f,  -0.5f, 0 }, // bottom right
    { -0.5f,   0.5f, 0 }, // top left
    {  0.5f,   0.5f, 0 }  // top right
};

typedef struct OpenglQuad {
    OpenglVertex vertex[6];
} OpenglQuad;

typedef struct OpenglTexture {
    V2 min, max;
    R2 dim;
    Bitmap *bitmap;
    b32 loaded;
} OpenglTexture;

#define TEXTURE_ATLAS_WIDTH 1024
#define TEXTURE_ATLAS_HEIGHT 1024
#define TEXTURE_ATLAS_DEFAULT_PADDING 4
#define MAX_ATLAS_TEXTURES 1024
struct OpenglGPU;

typedef struct OpenglTextureAtlas {
    u32 id;

    u32 buckets[MAX_ATLAS_TEXTURES];
    OpenglTexture textures[MAX_ATLAS_TEXTURES];
    u32 texture_count;

    i32 current_x;
    i32 current_y;
    i32 last_row_added_height;

    i32 w, h;

    u32 regeneration_count;
} OpenglTextureAtlas;

void texture_atlas_init(OpenglTextureAtlas *atlas);
OpenglTexture *texture_atlas_add_bitmap(struct OpenglGPU *renderer, Arena *arena, OpenglTextureAtlas *atlas, Bitmap *bitmap);
void texture_atlas_regenerate(Arena *arena, OpenglTextureAtlas *atlas);

typedef struct OpenglFrameBuffer {
    u32 id;
    u32 texture;
} OpenglFrameBuffer;

#define MAX_QUADS_PER_BATCH 1024
typedef struct OpenglGPU {
    Arena *arena;
    unsigned int program;
    unsigned int vao, vbo;
    OpenglQuad quad_buffer[MAX_QUADS_PER_BATCH];
    u32 quad_count;
    OpenglTextureAtlas atlas;
    b32 atlas_need_to_be_regenerate;
    u32 draw_calls;
} OpenglGPU;

void quad_batch_flush(OpenglGPU *renderer);
void quad_batch_push(OpenglGPU *renderer, OpenglQuad quad);
unsigned int gpu_create_program(const char *vert_src, const char *frag_src);

#endif //EDF_ANDROID_OPENGL_H
