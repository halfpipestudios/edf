//
// Created by tomas on 21/6/2024.
//

#include "android_opengl.h"
#include "android.h"

#include <unistd.h>

OpenglTexture *texture_atlas_add_bitmap(OpenglGPU *renderer, Arena *arena, OpenglTextureAtlas *atlas, Bitmap *bitmap) {

    assert(atlas->texture_count <= array_len(atlas->textures));

    OpenglTexture *texture = atlas->textures + atlas->texture_count;
    texture->bitmap = bitmap;
    texture->dim = r2_set_invalid();
    texture->loaded = false;

    // NOTE: insert the texture in sorted order

    u32 start = 0;
    u32 end = atlas->texture_count;
    u32 mid = start + (end - start)/2;
    u32 range = end - start;
    OpenglTexture *other_texture = atlas->textures + atlas->buckets[mid];

    while(range > 1) {
        if(texture->bitmap->h > other_texture->bitmap->h) {
            end = mid;
            mid = start + (end - start)/2;
        } else {
            start = mid;
            mid = start + (end - start)/2;
        }
        range = end - start;
        other_texture = atlas->textures + atlas->buckets[mid];
    }

    if(other_texture->bitmap->h > texture->bitmap->h) {
        mid++;
    }

#if 0
    memmove((atlas->buckets + (mid+1)), atlas->buckets + mid, (atlas->texture_count - mid)*sizeof(u32));
#else
    for(u32 i = atlas->texture_count; i > mid; --i) {
        assert(i > 0);
        atlas->buckets[i] = atlas->buckets[i-1];
    }
#endif

    u32 *bucket = atlas->buckets + mid;
    *bucket = atlas->texture_count++;

    renderer->atlas_need_to_be_regenerate = true;

    return texture;
}

void texture_atlas_regenerate(Arena *arena, OpenglTextureAtlas *atlas) {

    atlas->current_x = 0;
    atlas->current_y = 0;
    atlas->last_row_added_height = 0;

    // NOTE: Clear textures
    TempArena tmp = temp_arena_begin(arena);
    void *empty = arena_push(arena, sizeof(u32)*atlas->w*atlas->h, 8);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas->w, atlas->h, GL_RGBA, GL_UNSIGNED_BYTE, empty);
    temp_arena_end(tmp);

    // NOTE: Set up first white pixel;
    u32 white = 0xffffffff;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)&white);
    atlas->current_x += (1 + TEXTURE_ATLAS_DEFAULT_PADDING);

    // NOTE: Copy textures
    for(u32 i = 0; i < atlas->texture_count; ++i) {

        OpenglTexture *texture = atlas->textures + atlas->buckets[i];
        texture->loaded = true;

        if(i == 0) {
            atlas->last_row_added_height = texture->bitmap->h;
        } else {
            i32 width_left = (i32)atlas->w - (i32)atlas->current_x;
            if(width_left < (i32)texture->bitmap->w) {
                i32 new_row_height = texture->bitmap->h + TEXTURE_ATLAS_DEFAULT_PADDING;
                atlas->current_x = 0;
                atlas->current_y += atlas->last_row_added_height;
                atlas->last_row_added_height = new_row_height;
            }
        }

        assert((atlas->current_y + texture->bitmap->h + TEXTURE_ATLAS_DEFAULT_PADDING) <= atlas->h);

        texture->dim = r2_from_wh(atlas->current_x, atlas->current_y, texture->bitmap->w, texture->bitmap->h);
        texture->min = (V2){(f32)texture->dim.min.x / (f32)atlas->w, (f32)texture->dim.min.y / (f32)atlas->h};
        texture->max = (V2){(f32)(texture->dim.max.x + 1) / (f32)atlas->w, ((f32)texture->dim.max.y + 1) / (f32)atlas->h};

        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        texture->dim.min.x, texture->dim.min.y,
                        r2_width(texture->dim), r2_height(texture->dim),
                        GL_RGBA, GL_UNSIGNED_BYTE, texture->bitmap->data);

        atlas->current_x += (texture->bitmap->w + TEXTURE_ATLAS_DEFAULT_PADDING);
    }

    if(gcs) {
        static u32 count = 0;
        cs_print(gcs, "atlas regeneration count: %d\n", ++count);
    }
}

void texture_atlas_init(OpenglTextureAtlas *atlas) {
    atlas->w  = TEXTURE_ATLAS_WIDTH;
    atlas->h  = TEXTURE_ATLAS_HEIGHT;
    atlas->current_x = TEXTURE_ATLAS_DEFAULT_PADDING;
    atlas->current_y = TEXTURE_ATLAS_DEFAULT_PADDING;

    glGenTextures(1, &atlas->id);
    glBindTexture(GL_TEXTURE_2D, atlas->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas->w, atlas->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

}

void quad_batch_flush(OpenglGPU *renderer) {
    if(renderer->quad_count == 0) return;
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)renderer->quad_count*(GLsizeiptr)sizeof(OpenglQuad), renderer->quad_buffer);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)renderer->quad_count*6);
    renderer->quad_count = 0;
    renderer->draw_calls++;
}

void quad_batch_push(OpenglGPU *renderer, OpenglQuad quad) {
    if(renderer->quad_count >= array_len(renderer->quad_buffer)) {
        quad_batch_flush(renderer);
    }
    renderer->quad_buffer[renderer->quad_count++] = quad;
    if(renderer->quad_count == array_len(renderer->quad_buffer)) {
        i32 break_here = 0;
    }
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