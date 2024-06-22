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

    return texture;
}

void texture_atlas_sort_textures_per_height(OpenglTextureAtlas *atlas) {

    for(u32 i = 0; i < atlas->texture_count; ++i) {

        for(u32 j = i; j < atlas->texture_count; ++j) {

            if(i == j) continue;

            u32 *bucket = atlas->buckets + i;
            OpenglTexture *texture = atlas->textures + (*bucket);
            u32 h = texture->bitmap->height;

            u32 *other_bucket = atlas->buckets + j;
            OpenglTexture *other_texture = atlas->textures + (*other_bucket);
            u32 other_h = other_texture->bitmap->height;

            if(h < other_h) {
                u32 temp = *bucket;
                *bucket = *other_bucket;
                *other_bucket = temp;
            }
        }
    }

    u32 break_here = 0;
    unused(break_here);

}

void texture_atlas_calculate_size_and_alloc(Arena *arena, OpenglTextureAtlas *atlas) {
    atlas->bitmap.width = TEXTURE_ATLAS_START_WIDTH;
    atlas->bitmap.height = 1;
    atlas->current_x = 1 + TEXTURE_ATLAS_DEFAULT_PADDING;


    for(u32 i = 0; i < atlas->texture_count; ++i) {

        u32 *bucket = atlas->buckets + i;
        OpenglTexture *texture = atlas->textures + (*bucket);
        Bitmap *bitmap = texture->bitmap;
        assert(bitmap);

        if(i == 0) {
            atlas->bitmap.height = bitmap->height + TEXTURE_ATLAS_DEFAULT_PADDING;
            atlas->last_row_added_height = atlas->bitmap.height;
        } else {
            i32 width_left = (i32)atlas->bitmap.width - (i32)atlas->current_x;
            if(width_left < (i32)bitmap->width) {
                u32 new_row_height = bitmap->height + TEXTURE_ATLAS_DEFAULT_PADDING;
                atlas->bitmap.height += new_row_height;
                atlas->current_x = 0;
                atlas->current_y += atlas->last_row_added_height;
                atlas->last_row_added_height = new_row_height;

            }
        }

        atlas->current_x += bitmap->width + TEXTURE_ATLAS_DEFAULT_PADDING;
    }

    atlas->bitmap.data = arena_push(arena, atlas->bitmap.width*atlas->bitmap.height*sizeof(u32), 8);
    atlas->current_x = 0;
    atlas->current_y = 0;
    atlas->last_row_added_height = 0;

    ((u32 *)atlas->bitmap.data)[0] = 0xffffffff;
    atlas->current_x = 1 + TEXTURE_ATLAS_DEFAULT_PADDING;
}

void texture_atlas_insert_textures(OpenglTextureAtlas *atlas) {

    for(u32 i = 0; i < atlas->texture_count; ++i) {

        u32 *bucket = atlas->buckets + i;
        OpenglTexture *texture = atlas->textures + (*bucket);
        Bitmap *bitmap = texture->bitmap;
        assert(bitmap);

        if(i == 0) {
            atlas->last_row_added_height = bitmap->height + TEXTURE_ATLAS_DEFAULT_PADDING;
        } else {
            i32 width_left = (i32)atlas->bitmap.width - (i32)atlas->current_x;
            if(width_left < (i32)bitmap->width) {
                u32 new_row_height = bitmap->height + TEXTURE_ATLAS_DEFAULT_PADDING;
                atlas->current_x = 0;
                atlas->current_y += atlas->last_row_added_height;
                atlas->last_row_added_height = new_row_height;
            }
        }

        texture->dim = r2_from_wh((i32)atlas->current_x, (i32)atlas->current_y, (i32)bitmap->width, (i32)bitmap->height);
        texture->min = (V2){(f32)texture->dim.min.x / (f32)atlas->bitmap.width, (f32)texture->dim.min.y / (f32)atlas->bitmap.height};
        texture->max = (V2){(f32)(texture->dim.max.x + 1) / (f32)atlas->bitmap.width, ((f32)texture->dim.max.y + 1) / (f32)atlas->bitmap.height};

        for(i32 y = 0; y < bitmap->height; ++y) {
            i32 yy = y + (i32)atlas->current_y;
            for(i32 x = 0; x < bitmap->width; ++x) {
                i32 xx = x + (i32)atlas->current_x;
                ((u32 *)atlas->bitmap.data)[yy*atlas->bitmap.width+xx] = ((u32 *)texture->bitmap->data)[y*bitmap->width+x];
            }
        }

        atlas->current_x += bitmap->width + TEXTURE_ATLAS_DEFAULT_PADDING;
    }
}

void texture_atlas_generate(Arena *arena, OpenglTextureAtlas *atlas) {
    texture_atlas_sort_textures_per_height(atlas);
    texture_atlas_calculate_size_and_alloc(arena, atlas);
    texture_atlas_insert_textures(atlas);

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