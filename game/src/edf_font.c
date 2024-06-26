//
// Created by tomas on 21/6/2024.
//

#include "edf_font.h"
#include "edf_memory.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

Font *font_load(Gpu gpu, Arena *arena, char *path, float size) {
    Font *result         = arena_push(arena, sizeof(*result), 8);
    result->glyphs_count = (FONT_CODEPOINT_RANGE_END - FONT_CODEPOINT_RANGE_START + 1);
    result->glyphs       = arena_push(arena, sizeof(*result->glyphs) * result->glyphs_count, 8);

    File file = os_file_read(arena, path);

    stbtt_InitFont(&result->stbfont, file.data, stbtt_GetFontOffsetForIndex(file.data, 0));
    stbtt_fontinfo *font = &result->stbfont;

    i32 ascent_i, decent_i, line_gap_i;
    stbtt_GetFontVMetrics(font, &ascent_i, &decent_i, &line_gap_i);

    float scale      = stbtt_ScaleForPixelHeight(font, size);
    result->scale    = scale;
    result->ascent   = (f32)ascent_i * scale;
    result->decent   = (f32)decent_i * scale;
    result->line_gap = (f32)line_gap_i * scale;

    for(i32 codepoint = FONT_CODEPOINT_RANGE_START; codepoint <= FONT_CODEPOINT_RANGE_END;
        ++codepoint) {

        Glyph *glyph = result->glyphs + (codepoint - FONT_CODEPOINT_RANGE_START);
        i32 advance_w_i, left_bearing_i;
        stbtt_GetCodepointHMetrics(font, codepoint, &advance_w_i, &left_bearing_i);
        glyph->advance_w    = (f32)advance_w_i * scale;
        glyph->left_bearing = (f32)left_bearing_i * scale;

        stbtt_GetCodepointBitmapBox(font, codepoint, scale, scale, &glyph->bounds.min.x,
                                    &glyph->bounds.min.y, &glyph->bounds.max.x,
                                    &glyph->bounds.max.y);

        glyph->bitmap =
            bitmap_empty(arena, r2_width(glyph->bounds), r2_height(glyph->bounds), sizeof(u32));

        sz used = arena->used;

        Bitmap bitmap8 =
            bitmap_empty(arena, r2_width(glyph->bounds), r2_height(glyph->bounds), sizeof(u8));
        stbtt_MakeCodepointBitmap(font, (unsigned char *)bitmap8.data, (i32)bitmap8.w,
                                  (i32)bitmap8.h, (i32)bitmap8.w, scale, scale, codepoint);

        bitmap_copy_u8_u32(arena, &bitmap8, &glyph->bitmap);

        arena->used = used;

        glyph->texture = gpu_texture_load(gpu, &glyph->bitmap);
    }

    return result;
}

void font_unload(Gpu gpu, Font *font) {
    unused(gpu);
    unused(font);
}

R2 font_size_text(Font *font, const char *text, f32 x, f32 y) {

    R2 result = r2_set_invalid();

    f32 current_x = (f32)x;
    sz len        = strlen(text);
    for(sz i = 0; i < len; ++i) {
        u32 index = ((i32)text[i] - FONT_CODEPOINT_RANGE_START);
        assert(index < font->glyphs_count);
        Glyph *glyph = font->glyphs + index;

        f32 pos_x = current_x + glyph->left_bearing;
        f32 pos_y = y - (f32)glyph->bounds.max.y;

        R2 glyph_rect = r2_from_wh((i32)pos_x, (i32)pos_y, glyph->bitmap.w, glyph->bitmap.h);
        if(i == 0) {
            result = glyph_rect;
        } else {
            result = r2_union(result, glyph_rect);
        }

        current_x += glyph->advance_w;

        if(text[i + 1]) {
            i32 kerning =
                stbtt_GetCodepointKernAdvance(&font->stbfont, (i32)text[i], (i32)text[i + 1]);
            f32 scale_kerning = (f32)kerning * font->scale;
            current_x += scale_kerning;
        }
    }

    return result;
}

void font_draw_text(Gpu gpu, Font *font, const char *text, f32 x, f32 y, V4 color) {
    
    f32 current_x = (f32)x;
    sz len        = strlen(text);
    for(sz i = 0; i < len; ++i) {
        u32 index = ((i32)text[i] - FONT_CODEPOINT_RANGE_START);
        assert(index < font->glyphs_count);
        Glyph *glyph = font->glyphs + index;

        f32 pos_x = current_x + ((f32)glyph->bitmap.w * 0.5f) + glyph->left_bearing;
        f32 pos_y = y + (((f32)glyph->bitmap.h * 0.5f)) - (f32)glyph->bounds.max.y;

        gpu_draw_quad_texture_tinted(gpu, pos_x, pos_y, (f32)glyph->bitmap.w, (f32)glyph->bitmap.h,
                                     0, glyph->texture, color);

        current_x += glyph->advance_w;

        if(text[i + 1]) {
            i32 kerning =
                stbtt_GetCodepointKernAdvance(&font->stbfont, (i32)text[i], (i32)text[i + 1]);
            f32 scale_kerning = (f32)kerning * font->scale;
            current_x += scale_kerning;
        }
    }
}
