//
// Created by tomas on 21/6/2024.
//

#include "android_opengl.h"
#include "android.h"

OpenglTexture *texture_atlas_add_bitmap(Arena *arena, OpenglTextureAtlas *atlas, Bitmap *bitmap) {

    assert(atlas->texture_count <= array_len(atlas->textures));

    u32 *bucket = atlas->buckets + atlas->texture_count;
    *bucket = atlas->texture_count;
    ++atlas->texture_count;

    OpenglTexture *texture = atlas->textures + (*bucket);

    texture->bitmap = bitmap;
    texture->dim = r2_set_invalid();

    texture_atlas_regenerate(arena, atlas);

    return texture;
}

void texture_atlas_sort_textures_per_height(OpenglTextureAtlas *atlas) {

    for(u32 i = 0; i < atlas->texture_count; ++i) {

        for(u32 j = i; j < atlas->texture_count; ++j) {

            if(i == j) continue;

            u32 *bucket = atlas->buckets + i;
            OpenglTexture *texture = atlas->textures + (*bucket);
            u32 h = texture->bitmap->h;

            u32 *other_bucket = atlas->buckets + j;
            OpenglTexture *other_texture = atlas->textures + (*other_bucket);
            u32 other_h = other_texture->bitmap->h;

            if(h < other_h) {
                u32 temp = *bucket;
                *bucket = *other_bucket;
                *other_bucket = temp;
            }
        }
    }
}

void texture_atlas_calculate_size_and_alloc(Arena *arena, OpenglTextureAtlas *atlas) {
    atlas->bitmap.w = TEXTURE_ATLAS_START_WIDTH;
    atlas->bitmap.h = 1;
    atlas->current_x = 1 + TEXTURE_ATLAS_DEFAULT_PADDING;
    atlas->current_y = 0;

    for(u32 i = 0; i < atlas->texture_count; ++i) {

        u32 *bucket = atlas->buckets + i;
        OpenglTexture *texture = atlas->textures + (*bucket);
        Bitmap *bitmap = texture->bitmap;
        assert(bitmap);

        if(i == 0) {
            atlas->bitmap.h = bitmap->h + TEXTURE_ATLAS_DEFAULT_PADDING;
            atlas->last_row_added_height = atlas->bitmap.h;
        } else {
            i32 width_left = (i32)atlas->bitmap.w - (i32)atlas->current_x;
            if(width_left < (i32)bitmap->w) {
                i32 new_row_height = bitmap->h + TEXTURE_ATLAS_DEFAULT_PADDING;
                atlas->bitmap.h += new_row_height;
                atlas->current_x = 0;
                atlas->current_y += atlas->last_row_added_height;
                atlas->last_row_added_height = new_row_height;

            }
        }

        atlas->current_x += bitmap->w + TEXTURE_ATLAS_DEFAULT_PADDING;
    }

    atlas->bitmap.data = arena_push(arena, atlas->bitmap.w*atlas->bitmap.h*sizeof(u32), 8);
    atlas->current_x = 0;
    atlas->current_y = 0;
    atlas->last_row_added_height = 0;

    ((u32 *)atlas->bitmap.data)[0] = 0xffffffff;
    atlas->current_x = 1 + TEXTURE_ATLAS_DEFAULT_PADDING;

    if(atlas->bitmap.h > atlas->last_h) {
        atlas->need_to_be_regenerated = true;
        atlas->last_h = atlas->bitmap.h;
    }
}

void texture_atlas_insert_textures(OpenglTextureAtlas *atlas) {

    if(atlas->bitmap.h >= 185) {
        i32 break_here = 0;
        unused(break_here);
    }

    for(u32 i = 0; i < atlas->texture_count; ++i) {

        u32 *bucket = atlas->buckets + i;
        OpenglTexture *texture = atlas->textures + (*bucket);
        Bitmap *bitmap = texture->bitmap;
        assert(bitmap);

        if(i == 0) {
            atlas->last_row_added_height = bitmap->h + TEXTURE_ATLAS_DEFAULT_PADDING;
        } else {
            i32 width_left = (i32)atlas->bitmap.w - (i32)atlas->current_x;
            if(width_left < (i32)bitmap->w) {
                i32 new_row_height = bitmap->h + TEXTURE_ATLAS_DEFAULT_PADDING;
                atlas->current_x = 0;
                atlas->current_y += atlas->last_row_added_height;
                atlas->last_row_added_height = new_row_height;
            }
        }

        texture->dim = r2_from_wh((i32)atlas->current_x, (i32)atlas->current_y, (i32)bitmap->w, (i32)bitmap->h);
        texture->min = (V2){(f32)texture->dim.min.x / (f32)atlas->bitmap.w, (f32)texture->dim.min.y / (f32)atlas->bitmap.h};
        texture->max = (V2){(f32)(texture->dim.max.x + 1) / (f32)atlas->bitmap.w, ((f32)texture->dim.max.y + 1) / (f32)atlas->bitmap.h};

        for(i32 y = 0; y < bitmap->h; ++y) {
            i32 yy = y + (i32)atlas->current_y;
            for(i32 x = 0; x < bitmap->w; ++x) {
                i32 xx = x + (i32)atlas->current_x;
                ((u32 *)atlas->bitmap.data)[yy*atlas->bitmap.w+xx] = ((u32 *)texture->bitmap->data)[y*bitmap->w+x];
            }
        }

        atlas->current_x += bitmap->w + TEXTURE_ATLAS_DEFAULT_PADDING;
    }
}

void texture_atlas_regenerate(Arena *arena, OpenglTextureAtlas *atlas) {
    TempArena  temp = temp_arena_begin(arena);

    texture_atlas_sort_textures_per_height(atlas);
    texture_atlas_calculate_size_and_alloc(arena, atlas);
    texture_atlas_insert_textures(atlas);

    if(atlas->need_to_be_regenerated) {
        atlas->need_to_be_regenerated = false;

        if(!atlas->id) {
            glDeleteTextures(1, &atlas->id);
        }

        glGenTextures(1, &atlas->id);
        glBindTexture(GL_TEXTURE_2D, atlas->id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas->bitmap.w, atlas->bitmap.h, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, atlas->bitmap.data);

        static u32 count = 0;
        logd("Game", "textures atlas regeneration count: %d\n", ++count);

    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas->bitmap.w, atlas->bitmap.h, GL_RGBA,
                        GL_UNSIGNED_BYTE, atlas->bitmap.data);
    };

    temp_arena_end(temp);
}

void quad_batch_flush(OpenglGPU *renderer) {
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)renderer->quad_count*(GLsizeiptr)sizeof(OpenglQuad), renderer->quad_buffer);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)renderer->quad_count*6);
    renderer->quad_count = 0;
    renderer->draw_calls++;
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