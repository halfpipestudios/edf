#include <stdlib.h>
#include <assert.h>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <edf.h>
#include <edf_platform.h>

#include "android.h"
#include "android_opengl.h"

static jobject *asset_manager_ref   = 0;
static AAssetManager *asset_manager = 0;
static Memory global_memory;

static R2 global_display_rect;
static R2 global_device_rect;

void os_print(char *message, ...) {
    va_list args;
    va_start(args, message);
    logv("Game", message, args);
    va_end(args);
}

R2 os_display_rect(void) {
    return global_display_rect;
}

R2 os_device_rect(void) {
    return global_device_rect;
}

File os_file_read(struct Arena *arena, char *path) {
    AAsset *asset = AAssetManager_open(asset_manager, path, AASSET_MODE_BUFFER);
    File file = {0};
    file.size = AAsset_getLength(asset);
    file.data = arena_push(arena, file.size+1, 8);
    memcpy(file.data, AAsset_getBuffer(asset), file.size);
    ((u8 *)file.data)[file.size] = 0;
    AAsset_close(asset);
    return file;
}

bool os_file_write(u8 *data, sz size, char *path) { return false; }

Spu spu_load(struct Arena *arena) { return 0; }
void spu_unload(Spu spu) {}
void spu_clear(Spu spu) {}
Sound spu_sound_add(Spu spu, Wave *wave, bool playing, bool looping) { return (Sound){0}; }
void spu_sound_remove(Spu spu, Sound sound) {}
void spu_sound_play(Spu spu, Sound sound) {}
void spu_sound_pause(Spu spu, Sound sound) {}
void spu_sound_restart(Spu spu, Sound sound) {}

Gpu gpu_load(struct Arena *arena) {

    const char *version = (const char *)glGetString(GL_VERSION);
    logd("Game", "OpenGL initialized: %s", version);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    OpenglGPU *renderer = arena_push(arena, sizeof(*renderer), 8);
    renderer->arena = arena;
    renderer->load_textures = true;

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

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(OpenglVertex), offset_of(OpenglVertex, color));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    gpu_camera_set(renderer, v3(0, 0, 0), 0);

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
    if(renderer->load_textures) {
        texture_atlas_generate(renderer->arena, &renderer->atlas);
        renderer->load_textures = false;
        logd("Game", "Total platform memory used: %zuKB\n", renderer->arena->used/1024);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 1.0f);

    glUseProgram(renderer->program);
    glBindVertexArray(renderer->vao);

    renderer->draw_calls = 0;
}

void gpu_frame_end(Gpu gpu) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;

    // draw texture atlas test
#if 0
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float angle = 0;

    float window_w = (f32)r2_width(os_display_rect());
    float window_h = (f32) r2_height(os_display_rect());

    float ratio = (f32)renderer->atlas.bitmap.h / (f32)renderer->atlas.bitmap.w;
    float render_scale = 1;
    float w = (f32)renderer->atlas.bitmap.w  * render_scale;
    float h = w * ratio;

    float padding = 64;

    float x = (-window_w/2) + (w/2) + padding;
    float y = (window_h/2) - (h/2) - padding;

    M4 translate = m4_translate(v3(x, y, 0));
    M4 rotate = m4_rotate_z(angle);
    M4 scale = m4_scale(v3(w, h, 1));
    M4 world = m4_mul(translate, m4_mul(rotate, scale));

    OpenglQuad quad;
    V2 uvs[array_len(quad.vertex)] = {
        {1, 1}, // bottom right
        {0, 1}, // bottom left
        {0, 0}, // top left
        {1, 1}, // bottom right
        {0, 0}, // bottom right
        {1, 0}  // top right
    };

    for(u32 i = 0; i < array_len(quad.vertex); ++i) {
        V3 vertex = m4_mul_v3(world, vertices[i]);
        quad.vertex[i].pos = v2(vertex.x, vertex.y);
        quad.vertex[i].color = v4(1, 1, 1, 1);
        quad.vertex[i].uvs = uvs[i];
    }
    quad_batch_push(renderer, quad);

    // -----------------------------------------------
#endif
    quad_batch_flush(renderer);

    cs_print(gcs, "Draw calls: %d\n", renderer->draw_calls);
}

Texture gpu_texture_load(Gpu gpu, Bitmap *bitmap) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    OpenglTexture *texture = texture_atlas_add_bitmap(renderer->arena, &renderer->atlas, bitmap);
    return (Texture)texture;
}

void gpu_texture_unload(Gpu gpu, Texture texture) {
    (void)gpu;
    (void)texture;
}

void gpu_draw_quad_color(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, V4 color) {
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

void gpu_draw_quad_texture_tinted(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, Texture texture, V4 color) {
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
        quad.vertex[i].color = color;
        quad.vertex[i].uvs = uvs[i];
    }
    quad_batch_push(renderer, quad);
}

void gpu_draw_quad_texture(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, Texture texture) {
    gpu_draw_quad_texture_tinted(gpu, x, y, w, h, angle, texture, v4(1, 1, 1, 1));
}

void gpu_resize(Gpu gpu, u32 w, u32 h) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    f32 hw = (f32)w*0.5f;
    f32 hh = (f32)h*0.5f;
    M4 projection = m4_ortho(-hw, hw, hh, -hh, 0, 100);
    glUseProgram(renderer->program);
    i32 location = glGetUniformLocation(renderer->program, "projection");
    glUniformMatrix4fv(location, 1, true, projection.m);
}

void gpu_camera_set(Gpu gpu, V3 pos, f32 angle) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    quad_batch_flush(renderer);

    M4 view = m4_identity();
    M4 translate = m4_translate(v3(-pos.x, -pos.y, -pos.z));
    M4 rotate = m4_rotate_z(-angle);
    view = m4_mul(translate, m4_mul(rotate, view));
    glUseProgram(renderer->program);
    i32 location = glGetUniformLocation(renderer->program, "view");
    glUniformMatrix4fv(location, 1, true, view.m);
}

void gpu_blend_state_set(Gpu gpu, GpuBlendState blend_state) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;

    quad_batch_flush(renderer);

    if(blend_state == GPU_BLEND_STATE_ALPHA) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else if(blend_state == GPU_BLEND_STATE_ADDITIVE) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    } else {
        assert(!"Invalid code path");
    }
}

Input input_from_java(JNIEnv *env, jint touches_count, jintArray  indices, jobjectArray touches) {
    Input input = { 0 };

    input.count = min(touches_count, MAX_TOUCHES);
    int *indices_arr = (*env)->GetIntArrayElements(env, indices, false);
    int len = MAX_TOUCHES;

    for(u32 i = 0; i < len; ++i) {
        jobject touch = (*env)->GetObjectArrayElement(env, touches, (jsize)i);

        jclass touch_class = (*env)->GetObjectClass(env, touch);
        jfieldID index_id = (*env)->GetFieldID(env, touch_class, "index", "I");
        jfieldID x_id  = (*env)->GetFieldID(env, touch_class, "x", "F");
        jfieldID y_id  = (*env)->GetFieldID(env, touch_class, "y", "F");

        input.touches[i].uid = (u64)(*env)->GetIntField(env, touch, index_id);
        input.touches[i].pos.x = (i32)(*env)->GetFloatField(env, touch, x_id);
        input.touches[i].pos.y = (i32)(*env)->GetFloatField(env, touch, y_id);

        input.locations[i] = indices_arr[i];
    }

    return input;
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameInit(JNIEnv *env, jobject thiz,  jobject manager) {
    (void)thiz;
    asset_manager_ref = (*env)->NewGlobalRef(env, manager);
    asset_manager     = AAssetManager_fromJava(env, asset_manager_ref);
    assert(asset_manager_ref);
    assert(asset_manager);

    global_memory.size = GAME_MEMORY_SIZE;
    global_memory.used = 0;
    global_memory.data = malloc(global_memory.size);
    game_init(&global_memory);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameUpdate(JNIEnv *env, jobject thiz, jint count, jintArray indices, jobjectArray touches, jfloat dt) {
    (void)env;
    (void)thiz;
    Input input = input_from_java(env, count, indices, touches);
    game_update(&global_memory, &input, dt);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameRender(JNIEnv *env, jobject thiz) {
    (void)env;
    (void)thiz;
    game_render(&global_memory);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameResize(JNIEnv *env, jobject thiz, jint x, jint y, jint w, jint h) {

    f32 ratio = ((f32)VIRTUAL_RES_Y / (f32)VIRTUAL_RES_X);
    i32 virtual_w = w;
    i32 virtual_h = (i32)((f32)w * ratio);

    if(virtual_h > h) {
        ratio = ((f32)VIRTUAL_RES_X / (f32)VIRTUAL_RES_Y);
        virtual_h = h;
        virtual_w = (i32)((f32)h * ratio);

    }

#if 0
    if(virtual_w > w) {
        f32 ratio_w = ((f32)VIRTUAL_RES_Y / (f32)VIRTUAL_RES_X);
        f32 ratio_h = ((f32)VIRTUAL_RES_X / (f32)VIRTUAL_RES_Y);
        virtual_h = (i32)((f32)w * ratio);
        virtual_w = (i32)((f32)h * ratio);
    }
#endif

    i32 pos_y = (i32)((f32)h*0.5f-(f32)virtual_h*0.5f);
    i32 pos_x = (i32)((f32)w*0.5f-(f32)virtual_w*0.5f);

    global_display_rect = r2_from_wh(pos_x, pos_y, virtual_w, virtual_h);
    global_device_rect = r2_from_wh(0, 0, w, h);

    glViewport(pos_x, pos_y, virtual_w, virtual_h);
    game_resize(&global_memory, virtual_w, virtual_h);
}


JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gpuSetViewport(JNIEnv *env, jobject thiz, jint x, jint y, jint w, jint h) {
    (void) env;
    (void) thiz;

}