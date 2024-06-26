//
// Created by tomas on 21/6/2024.
//

#ifndef EDF_EDF_FONT_H
#define EDF_EDF_FONT_H

#include "edf_platform.h"

#include "stb_truetype.h"

#define FONT_CODEPOINT_RANGE_START ((i32)' ')
#define FONT_CODEPOINT_RANGE_END ((i32)'~')

typedef struct Glyph {
    Texture texture;
    Bitmap bitmap;
    R2 bounds;

    f32 advance_w;
    f32 left_bearing;

} Glyph;

typedef struct Font {
    u32 size;

    Glyph *glyphs;
    u32 glyphs_count;

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

R2 font_size_text(Font *font, const char *text);
void font_draw_text(Gpu gpu, Font *font, const char *text, f32 x, f32 y, V4 color);

#endif // EDF_EDF_FONT_H
