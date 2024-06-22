//
// Created by tomas on 21/6/2024.
//

#ifndef EDF_EDF_FONT_H
#define EDF_EDF_FONT_H

#include <edf_platform.h>

typedef struct Glyph {
    Texture texture;
    Bitmap bitmap;
    i32 codepoint;
} Glyph;

typedef struct Font {
    Glyph *glyphs;
    u32 glyphs_count;
} Font;

Font *font_load(Gpu gpu, struct Arena *arena, char *path);
void font_unload(Gpu gpu, Font *font);

#endif //EDF_EDF_FONT_H
