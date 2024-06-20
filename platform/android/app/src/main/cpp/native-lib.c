#include <stdlib.h>
#include <assert.h>

#include <jni.h>
#include <GLES3/gl3.h>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <edf.h>
#include <edf_platform.h>

#define logd(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)

static jobject *asset_manager_ref   = 0;
static AAssetManager *asset_manager = 0;
static Memory global_memory;

File os_file_read(struct Arena *arena, char *path) {
    AAsset *asset = AAssetManager_open(asset_manager, path, AASSET_MODE_BUFFER);
    File file = {0};
    file.size = AAsset_getLength(asset);
    file.data = arena_push(arena, file.size+1, 8);
    memcpy(file.data, AAsset_getBuffer(asset), file.size);
    ((u8 *)file.data)[file.size] = 0;
    return file;
}

#define MAX_QUADS_PER_BATCH 1024

typedef struct OpenglVertex {
    V2 pos;
    V2 uvs;
    V3 color;
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
    u32 index;
    V2 min, max;
    R2 dim;
    Bitmap *bitmap;
} OpenglTexture;

#define TEXTURE_ATLAS_START_WIDTH 1024
#define TEXTURE_ATLAS_DEFAULT_PADDING 4
#define MAX_ATLAS_TEXTURES 256

typedef struct OpenglTextureAtlas {
    u32 id;
    Bitmap bitmap;
    u32 current_x;
    u32 current_y;
    u32 last_row_added_height;
    OpenglTexture textures[MAX_ATLAS_TEXTURES];
    u32 texture_count;
} OpenglTextureAtlas;

OpenglTexture *texture_atlas_add_bitmap(OpenglTextureAtlas *atlas, Bitmap *bitmap) {

    assert(atlas->texture_count <= array_len(atlas->textures));

    u32 index = atlas->texture_count;
    OpenglTexture *texture = atlas->textures + index;
    ++atlas->texture_count;

    texture->bitmap = bitmap;
    texture->dim = r2_set_invalid();
    texture->index = index;

    return texture;
}

void texture_atlas_sort_textures_per_height(OpenglTextureAtlas *atlas) {

    for(u32 i = 0; i < atlas->texture_count; ++i) {

        OpenglTexture *texture = atlas->textures + i;
        u32 h = texture->bitmap->height;

        for(u32 j = i; j < atlas->texture_count; ++j) {

            if(i == j) continue;

            OpenglTexture *other_texture = atlas->textures + j;
            u32 other_h = other_texture->bitmap->height;

            if(h < other_h) {
                OpenglTexture temp = *texture;
                *texture = *other_texture;
                *other_texture = temp;

                u32 temp_index = texture->index;
                texture->index = other_texture->index;
                other_texture->index = temp_index;
            }
        }
    }
}

void texture_atlas_calculate_size_and_alloc(Arena *arena, OpenglTextureAtlas *atlas) {
    for(u32 i = 0; i < atlas->texture_count; ++i) {

        OpenglTexture *texture = atlas->textures + i;
        Bitmap *bitmap = texture->bitmap;
        assert(bitmap);

        if(i == 0) {
            atlas->last_row_added_height = bitmap->height + TEXTURE_ATLAS_DEFAULT_PADDING;
            atlas->bitmap.height = atlas->last_row_added_height;
            atlas->bitmap.width = TEXTURE_ATLAS_START_WIDTH;
            atlas->current_x = 1 + TEXTURE_ATLAS_DEFAULT_PADDING;
        }

        u32 width_left = atlas->bitmap.width - atlas->current_x;
        if(width_left < (i32)bitmap->width) {

            u32 new_row_height = bitmap->height + TEXTURE_ATLAS_DEFAULT_PADDING;
            atlas->bitmap.height = atlas->bitmap.width;
            atlas->bitmap.width = atlas->bitmap.height + new_row_height;
            atlas->current_x = 0;
            atlas->current_y += atlas->last_row_added_height;
            atlas->last_row_added_height = new_row_height;

        }

        atlas->current_x += bitmap->width + TEXTURE_ATLAS_DEFAULT_PADDING;
    }

    atlas->bitmap.data = arena_push(arena, atlas->bitmap.width*atlas->bitmap.height*sizeof(u32), 8);
    atlas->current_x = 0;
    atlas->current_y = 0;
    atlas->last_row_added_height = 0;

    atlas->bitmap.data[0] = 0xffffffff;
    atlas->current_x = 1 + TEXTURE_ATLAS_DEFAULT_PADDING;
}

void texture_atlas_insert(OpenglTextureAtlas *atlas, OpenglTexture *texture) {
    Bitmap *texture_atlas_bitmap = &atlas->bitmap;
    Bitmap *bitmap = texture->bitmap;
    assert(bitmap);

    u32 width_left = atlas->bitmap.width - atlas->current_x;
    if(width_left < (i32)bitmap->width) {

        u32 new_row_height = bitmap->height + TEXTURE_ATLAS_DEFAULT_PADDING;
        atlas->bitmap.height = atlas->bitmap.width;
        atlas->bitmap.width = atlas->bitmap.height + new_row_height;
        atlas->current_x = 0;
        atlas->current_y += atlas->last_row_added_height;
        atlas->last_row_added_height = new_row_height;

    }

    texture->dim = r2_from_wh((i32)atlas->current_x, (i32)atlas->current_y, (i32)bitmap->width, (i32)bitmap->height);
    texture->min = (V2){(f32)texture->dim.min.x / (f32)atlas->bitmap.width, (f32)texture->dim.min.y / (f32)atlas->bitmap.height};
    texture->max = (V2){(f32)(texture->dim.max.x + 1) / (f32)atlas->bitmap.width, ((f32)texture->dim.max.y + 1) / (f32)atlas->bitmap.height};

    for(i32 y = 0; y < bitmap->height; ++y) {
        i32 yy = y + (i32)atlas->current_y;
        for(i32 x = 0; x < bitmap->width; ++x) {
            i32 xx = x + (i32)atlas->current_x;
            atlas->bitmap.data[yy*atlas->bitmap.width+xx] = texture->bitmap->data[y*bitmap->width+x];
        }
    }

    atlas->current_x += bitmap->width + TEXTURE_ATLAS_DEFAULT_PADDING;
}

void texture_atlas_generate(Arena *arena, OpenglTextureAtlas *atlas) {
    texture_atlas_sort_textures_per_height(atlas);
    texture_atlas_calculate_size_and_alloc(arena, atlas);
    for(u32 i = 0; i < atlas->texture_count; ++i) {
        OpenglTexture *texture = atlas->textures + i;
        texture_atlas_insert(atlas, texture);
    }

    glGenTextures(1, &atlas->id);
    glBindTexture(GL_TEXTURE_2D, atlas->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (i32)atlas->bitmap.width, (i32)atlas->bitmap.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, atlas->bitmap.data);
}

typedef struct OpenglGPU {
    Arena *arena;
    unsigned int program;
    unsigned int vao, vbo;
    OpenglQuad quad_buffer[MAX_QUADS_PER_BATCH];
    u32 quad_count;
    OpenglTextureAtlas atlas;
    b32 first_frame;
} OpenglGPU;

void quad_batch_flush(OpenglGPU *renderer) {
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)renderer->quad_count*(GLsizeiptr)sizeof(OpenglQuad), renderer->quad_buffer);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)renderer->quad_count*6);
    renderer->quad_count = 0;
}

void quad_batch_push(OpenglGPU *renderer, OpenglQuad quad) {
    if(renderer->quad_count > array_len(renderer->quad_buffer)) {
        quad_batch_flush(renderer);
    }
    renderer->quad_buffer[renderer->quad_count++] = quad;
}

unsigned int gpu_create_program(const char *vert_src, const char *frag_src) {

    unsigned int vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_src, 0);
    glCompileShader(vert_shader);
    int success;
    char info[512];
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vert_shader, 512, 0, info);
        logd("Game", "[Vertex Shader] %s\n", info);
    }

    unsigned int frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_src, 0);
    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(frag_shader, 512, 0, info);
        logd("Game", "[Fragment Shader] %s\n", info);
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, 0, info);
        logd("Game", "[Program] %s\n", info);
    }
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return program;
}

Gpu gpu_load(struct Arena *arena) {

    const char *version = (const char *)glGetString(GL_VERSION);
    logd("Game", "OpenGL initialized: %s", version);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    OpenglGPU *renderer = arena_push(arena, sizeof(*renderer), 8);
    renderer->arena = arena;
    renderer->first_frame = true;

    File vert_src = os_file_read(arena, "shader.vert");
    File frag_src = os_file_read(arena, "shader.frag");

    renderer->program = gpu_create_program((const char *)vert_src.data, (const char *)frag_src.data);

    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);

    glBindVertexArray(renderer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(OpenglQuad)*MAX_QUADS_PER_BATCH, 0, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(OpenglVertex), offset_of(OpenglVertex, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(OpenglVertex), offset_of(OpenglVertex, uvs));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(OpenglVertex), offset_of(OpenglVertex, color));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return (Gpu *)renderer;
}

void gpu_unload(Gpu gpu) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    glDeleteProgram(renderer->program);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteVertexArrays(1, &renderer->vao);
    glDeleteTextures(1, &renderer->atlas.id);
}

void gpu_frame_begin(Gpu gpu) {

    OpenglGPU *renderer = (OpenglGPU *)gpu;

    if(renderer->first_frame) {
        texture_atlas_generate(renderer->arena, &renderer->atlas);
        renderer->first_frame = false;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0.5f, 1, 1.0f);

    glUseProgram(renderer->program);
    glBindVertexArray(renderer->vao);
}

void gpu_frame_end(Gpu gpu) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    quad_batch_flush(renderer);
}

Texture gpu_texture_load(Gpu gpu, Bitmap *bitmap) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    OpenglTexture *texture = texture_atlas_add_bitmap(&renderer->atlas, bitmap);
    return (Texture)texture;
}

void gpu_texture_unload(Gpu gpu, Texture texture) {
    (void)gpu;
    (void)texture;
}

void gpu_draw_quad_color(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, V3 color) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;

    M4 translate = m4_translate(v3(x, y, 0));
    M4 rotate = m4_rotate_z(angle);
    M4 scale = m4_scale(v3(w, h, 1));
    M4 world = m4_mul(translate, m4_mul(rotate, scale));

    OpenglQuad quad;
    for(u32 i = 0; i < array_len(quad.vertex); ++i) {
        V3 vertex = m4_mul_v3(world, vertices[i]);
        quad.vertex[i].pos = v2(vertex.x, vertex.y);
        quad.vertex[i].color = color;
        quad.vertex[i].uvs = v2(0, 0);
    }
    quad_batch_push(renderer, quad);
}

void gpu_draw_quad_texture(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, Texture texture) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;

    M4 translate = m4_translate(v3(x, y, 0));
    M4 rotate = m4_rotate_z(angle);
    M4 scale = m4_scale(v3(w, h, 1));
    M4 world = m4_mul(translate, m4_mul(rotate, scale));

    OpenglTexture *tex = (OpenglTexture *)texture;
    OpenglQuad quad;
    V2 uvs[array_len(quad.vertex)] = {
        {tex->max.x, tex->max.y}, // bottom right
        {tex->min.x, tex->max.y}, // bottom left
        {tex->min.x, tex->min.y}, // top left
        {tex->max.x, tex->max.y}, // bottom right
        {tex->min.x, tex->min.y}, // bottom right
        {tex->max.x, tex->min.y}  // top right
    };

    for(u32 i = 0; i < array_len(quad.vertex); ++i) {
        V3 vertex = m4_mul_v3(world, vertices[i]);
        quad.vertex[i].pos = v2(vertex.x, vertex.y);
        quad.vertex[i].color = v3(1, 1, 1);
        quad.vertex[i].uvs = uvs[i];
    }
    quad_batch_push(renderer, quad);
}

void gpu_resize(Gpu gpu, u32 w, u32 h) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    glViewport(0, 0, (i32)w, (i32)h);
    f32 hw = (f32)w*0.5f;
    f32 hh = (f32)h*0.5f;
    M4 projection = m4_ortho(-hw, hw, hh, -hh, 0, 100);
    glUseProgram(renderer->program);
    i32 location = glGetUniformLocation(renderer->program, "projection");
    glUniformMatrix4fv(location, 1, true, projection.m);
}


JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameInit(JNIEnv *env, jobject thiz, jobject manager) {
    (void)thiz;
    asset_manager_ref = (*env)->NewLocalRef(env, manager);
    asset_manager     = AAssetManager_fromJava(env, asset_manager_ref);
    assert(asset_manager_ref);
    assert(asset_manager);
    global_memory.size = GAME_MEMORY_SIZE;
    global_memory.used = 0;
    global_memory.data = malloc(global_memory.size);
    game_init(&global_memory);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameUpdate(JNIEnv *env, jobject thiz, jfloat dt) {
    (void)env;
    (void)thiz;
    game_update(&global_memory, 0, dt);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameRender(JNIEnv *env, jobject thiz) {
    (void)env;
    (void)thiz;

    game_render(&global_memory);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gpuSetViewport(JNIEnv *env, jobject thiz, jint x, jint y, jint w, jint h) {
    (void) env;
    (void) thiz;
    game_resize(&global_memory, w, h);
}