//
// Created by tomas on 21/6/2024.
//

#ifndef EDF_EDF_FONT_H
#define EDF_EDF_FONT_H

#include "edf_graphics.h"
#include "edf_memory.h"
#include "edf_platform.h"

#include "stb_truetype.h"

#define FONT_CODEPOINT_RANGE_START ((i32)' ')
#define FONT_CODEPOINT_RANGE_END ((i32)'~')

typedef struct Glyph {
    u32 code;

    Texture texture;
    Bitmap bitmap;
    R2 bounds;

    f32 advance_w;
    f32 left_bearing;
    
} Glyph;

typedef struct Font {
    Arena *arena;

    Glyph *glyphs;
    u32 glyphs_count;

    #define MAX_GLYPH_CACHE_SIZE 256
    Glyph glyphs_cahce[MAX_GLYPH_CACHE_SIZE];
    u32 glyphs_cache_count;
    
    u32 size;
    f32 ascent;
    f32 decent;
    f32 line_gap;

    f32 scale;
    stbtt_fontinfo stbfont;

} Font;

typedef struct FontInfo {
    #define MAX_FONT_INFO_NAME 256
    char name[MAX_FONT_INFO_NAME];
    stbtt_fontinfo info;
} FontInfo;

FontInfo font_info_load(struct Arena *arena, char *path);

Font *font_load(Gpu gpu, struct Arena *arena, stbtt_fontinfo stbfont, float size);
void font_unload(Gpu gpu, Font *font);

R2 font_size_text(Gpu gpu, Font *font, const char *text);
void font_draw_text(Gpu gpu, Font *font, const char *text, f32 x, f32 y, V4 color);

void font_rasterize_glyph(Gpu gpu, Font *font, Glyph *glyph, u32 code);
Glyph *font_get_glyph(Gpu gpu, Font *font, u32 code);
Glyph *font_find_glyph_in_chache(Font *font, u32 code);


#endif // EDF_EDF_FONT_H
