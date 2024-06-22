//
// Created by tomas on 21/6/2024.
//

#include "edf_font.h"
#include "edf_memory.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

Font *font_load(Gpu gpu, Arena *arena, char *path) {
    Font *result = arena_push(arena, sizeof(*result), 8);
    result->glyphs_count =  (FONT_CODEPOINT_RANGE_END - FONT_CODEPOINT_RANGE_START + 1);
    result->glyphs = arena_push(arena, sizeof(*result->glyphs)*result->glyphs_count, 8);

    File file = os_file_read(arena, path);

    stbtt_InitFont(&result->stbfont, file.data, stbtt_GetFontOffsetForIndex(file.data, 0));
    stbtt_fontinfo *font = & result->stbfont;

    i32 ascent_i, decent_i, line_gap_i;
    stbtt_GetFontVMetrics(font, &ascent_i, &decent_i, &line_gap_i);

    float scale = stbtt_ScaleForPixelHeight(font, 64*2);
    result->scale = scale;
    result->ascent = (f32)ascent_i * scale;
    result->decent = (f32)decent_i * scale;
    result->line_gap = (f32)line_gap_i * scale;

    for(i32 codepoint = FONT_CODEPOINT_RANGE_START; codepoint <= FONT_CODEPOINT_RANGE_END; ++codepoint) {

        Glyph *glyph = result->glyphs + (codepoint - FONT_CODEPOINT_RANGE_START);
        i32 advance_w_i, left_bearing_i;
        stbtt_GetCodepointHMetrics(font, codepoint, &advance_w_i, &left_bearing_i);
        glyph->advance_w = (f32)advance_w_i * scale;
        glyph->left_bearing = (f32)left_bearing_i * scale;

        stbtt_GetCodepointBitmapBox(font, codepoint, scale, scale, &glyph->bounds.min.x, &glyph->bounds.min.y, &glyph->bounds.max.x, &glyph->bounds.max.y);

        glyph->bitmap = bitmap_empty(arena, r2_width(glyph->bounds), r2_height(glyph->bounds), sizeof(u32));

        sz used = arena->used;

        Bitmap bitmap8 = bitmap_empty(arena, r2_width(glyph->bounds), r2_height(glyph->bounds), sizeof(u8));
        stbtt_MakeCodepointBitmap(font, (unsigned char *)bitmap8.data,
                                  (i32)bitmap8.width, (i32)bitmap8.height, (i32)bitmap8.width, scale, scale, codepoint);

        bitmap_copy_u8_u32(arena, &bitmap8, &glyph->bitmap);

        arena->used = used;

        glyph->texture = gpu_texture_load(gpu, &glyph->bitmap);
    }

    return result;
}

void font_unload(Gpu gpu, Font *font) {
    unused(font);
}

void font_draw_text(Gpu gpu, Font *font, const char *text, f32 x, f32 y, V3 color) {

    f32 current_x = (f32)x;

    sz len = strlen(text);
    for(sz i = 0; i < len; ++i) {
        u32 index = ((i32)text[i] - FONT_CODEPOINT_RANGE_START);
        assert(index < font->glyphs_count);
        Glyph *glyph = font->glyphs + index;


        f32 pos_x = current_x + ((f32)glyph->bitmap.width*0.5f) + glyph->left_bearing;
        f32 pos_y = y + (((f32)glyph->bitmap.height*0.5f)) - (f32)glyph->bounds.max.y;

        gpu_draw_quad_texture(gpu, pos_x, pos_y, (f32)glyph->bitmap.width, (f32)glyph->bitmap.height, 0, glyph->texture);

        current_x += glyph->advance_w;

        if (text[index+1]) {
            current_x += (f32)stbtt_GetCodepointKernAdvance(&font->stbfont, text[index],text[index+1]) * font->scale;
        }

    }
}
