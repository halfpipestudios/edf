//
// Created by tomas on 21/6/2024.
//

#ifndef EDF_EDF_FONT_H
#define EDF_EDF_FONT_H

#include <edf_platform.h>

#define FONT_CODEPOINT_RANGE_START ((i32)' ')
#define FONT_CODEPOINT_RANGE_END ((i32)'~')

typedef struct Glyph {
    Texture texture;
    Bitmap bitmap;

    f32 advance_w;
    f32 left_bearing;

} Glyph;

typedef struct Font {
    Glyph *glyphs;
    u32 glyphs_count;

    f32 ascent;
    f32 decent;
    f32 line_gap;

} Font;

Font *font_load(Gpu gpu, struct Arena *arena, char *path);
void font_unload(Gpu gpu, Font *font);

void font_draw_text(Gpu gpu, Font *font, const char *text, i32 x, i32 y, V3 color);

#endif //EDF_EDF_FONT_H
