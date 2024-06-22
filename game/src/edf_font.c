//
// Created by tomas on 21/6/2024.
//

#include "edf_font.h"
#include "edf_memory.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

Font *font_load(Gpu gpu, Arena *arena, char *path) {
    File file = os_file_read(arena, path);

    stbtt_fontinfo font;
    stbtt_InitFont(&font, file.data, stbtt_GetFontOffsetForIndex(file.data,0));

    float scale = stbtt_ScaleForPixelHeight(&font, 64);

    for(i32 codepoint = (i32)' '; codepoint <= (i32)'~'; ++codepoint) {
        i32 x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&font, codepoint, scale, scale, &x0, &y0, &x1, &y1);

        Bitmap bitmap8 = bitmap_empty(arena, x1 - x0 + 1, y1 - y0 + 1, sizeof(u8));
        stbtt_MakeCodepointBitmap(&font, (unsigned char *)bitmap8.data,
                                  (i32)bitmap8.width, (i32)bitmap8.height, (i32)bitmap8.width, scale, scale, codepoint);

        Bitmap bitmap32 = bitmap_copy_u8_u32(arena, &bitmap8);
        if(bitmap32.width != 0 && bitmap32.height != 0) {
            Texture font_texture = gpu_texture_load(gpu, &bitmap32);
        }
    }

    return 0;
}

void font_unload(Gpu gpu, Font *font) {
    unused(font);
}