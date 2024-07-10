#include <stdlib.h>
#include <assert.h>

#include <android/asset_manager.h>

#include "android_platform.h"
#include "android_opengl.h"

// -------------------------------
// OS functions implementation
// -------------------------------

void os_print(char *message, ...) {
    va_list args;
    va_start(args, message);
    logv("Game", message, args);
    va_end(args);
}

File os_file_read(struct Arena *arena, char *path) {
    AAsset *asset = AAssetManager_open(asset_manager, path, AASSET_MODE_BUFFER);
    assert(asset);
    File file = {0};
    file.size = AAsset_getLength(asset);
    file.data = arena_push(arena, file.size+1, 8);
    memcpy(file.data, AAsset_getBuffer(asset), file.size);
    ((u8 *)file.data)[file.size] = 0;
    AAsset_close(asset);
    return file;
}

bool os_file_write(u8 *data, sz size, char *path) { return false; }

// -------------------------------
// SPU functions implementation
// -------------------------------

Spu spu_load(struct Arena *arena) { return 0; }
void spu_unload(Spu spu) {}
void spu_clear(Spu spu) {}
Sound spu_sound_add(Spu spu, Wave *wave, bool playing, bool looping) { return (Sound){0}; }
void spu_sound_remove(Spu spu, Sound sound) {}
void spu_sound_play(Spu spu, Sound sound) {}
void spu_sound_pause(Spu spu, Sound sound) {}
void spu_sound_restart(Spu spu, Sound sound) {}

// -------------------------------
// GPU functions implementation
// -------------------------------

Gpu gpu_load(struct Arena *arena) {

    const char *version = (const char *)glGetString(GL_VERSION);
    logd("Game", "OpenGL initialized: %s", version);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    OpenglGPU *renderer = arena_push(arena, sizeof(*renderer), 8);
    renderer->arena = arena;
    texture_atlas_init(&renderer->atlas);

    TempArena temp = temp_arena_begin(arena);

    File vert_src = os_file_read(temp.arena, "shader.vert");
    File frag_src = os_file_read(temp.arena, "shader.frag");

    renderer->program = create_program((const char *)vert_src.data, (const char *)frag_src.data);

    temp_arena_end(temp);

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

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 1.0f);

    glUseProgram(renderer->program);
    glBindVertexArray(renderer->vao);

    renderer->draw_calls = 0;
}

void gpu_frame_end(Gpu gpu) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;

    //draw_texture_atlas(renderer);
    quad_batch_flush(renderer);

    if(renderer->atlas_need_to_be_regenerate) {
        texture_atlas_regenerate(renderer->arena, &renderer->atlas);
        renderer->atlas_need_to_be_regenerate = false;
    }
    //cs_print(gcs, "Draw calls: %d\n", renderer->draw_calls);
}

Texture gpu_texture_load(Gpu gpu, Bitmap *bitmap) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    OpenglTexture *texture = texture_atlas_add_bitmap(renderer, renderer->arena, &renderer->atlas, bitmap);
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
    OpenglTexture *tex = (OpenglTexture *)texture;
    if(!tex->loaded) {
        return;
    }

    M4 translate = m4_translate(v3(x, y, 0));
    M4 rotate = m4_rotate_z(angle);
    M4 scale = m4_scale(v3(w, h, 1));
    M4 world = m4_mul(translate, m4_mul(rotate, scale));

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

void gpu_viewport_set(Gpu gpu, f32 x, f32 y, f32 w, f32 h) {
    glViewport((i32)x, (i32)y, (i32)w, (i32)h);
}

void gpu_projection_set(Gpu gpu, f32 l, f32 r, f32 t, f32 b) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    quad_batch_flush(renderer);

    M4 projection = m4_ortho(l, r, t, b, 0, 100);
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

RenderTarget gpu_render_targte_load(Gpu gpu, i32 width, i32 height) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;

    OpenglFrameBuffer *frame_buffer = (OpenglFrameBuffer *) arena_push(renderer->arena, sizeof(*frame_buffer), 8);
    glGenFramebuffers(1, &frame_buffer->id);

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer->id);

    glGenTextures(1, &frame_buffer->texture);
    glBindTexture(GL_TEXTURE_2D, frame_buffer->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_buffer->texture, 0);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, renderer->atlas.id);
    return (RenderTarget *)frame_buffer;
}

void gpu_render_target_begin(Gpu gpu, RenderTarget rt) {
    OpenglFrameBuffer *frame_buffer = (OpenglFrameBuffer *)rt;
    if(frame_buffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer->id);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void gpu_render_target_end(Gpu gpu, RenderTarget rt) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    quad_batch_flush(renderer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gpu_render_target_draw(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, RenderTarget rt) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    OpenglFrameBuffer *frame_buffer = (OpenglFrameBuffer *)rt;
    glBindTexture(GL_TEXTURE_2D, frame_buffer->texture);

    M4 translate = m4_translate(v3(x, y, 0));
    M4 rotate = m4_rotate_z(angle);
    M4 scale = m4_scale(v3(w, h, 1));
    M4 world = m4_mul(translate, m4_mul(rotate, scale));

    OpenglQuad quad;
    V2 uvs[array_len(quad.vertex)] = {
            {1, 0}, // bottom right
            {0, 0}, // bottom left
            {0, 1}, // top left
            {1, 0}, // bottom right
            {0, 1}, // bottom right
            {1, 1}  // top right
    };

    for(u32 i = 0; i < array_len(quad.vertex); ++i) {
        V3 vertex = m4_mul_v3(world, vertices[i]);
        quad.vertex[i].pos = v2(vertex.x, vertex.y);
        quad.vertex[i].color = v4(1, 1, 1, 1);
        quad.vertex[i].uvs = uvs[i];
    }

    quad_batch_push(renderer, quad);
    quad_batch_flush(renderer);

    glBindTexture(GL_TEXTURE_2D, renderer->atlas.id);
}