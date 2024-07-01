#include "edf_platform.h"
#include "edf_debug.h"
#include "edf_memory.h"
#include "edf_font.h"

#include <stdarg.h>
#include <stdio.h>

static void cs_print_text(Console *c, char *text) {
    u32 len = strlen(text);

    for(u32 i = 0; i < len; ++i) {
        
        char character = text[i];
        if(character == '\n') {
            c->col = 0;
            c->line = (c->line + 1) % c->max_rows;
            continue;
        }

        if(c->col >= c->max_cols) {
            c->col = 0;
            c->line = (c->line + 1) % c->max_rows;
        }

        if(c->line == c->next_line) {
            memset(&c->buffer[c->line * c->max_cols], 0, c->max_cols);
            c->next_line = (c->next_line + 1) % c->max_rows;
        }

        c->buffer[c->line * c->max_cols + c->col++] = character;
    }
}

void cs_print(Console *c, char *format, ...) {
    va_list args;
    va_start(args, format);
    Arena *arena = get_scratch_arena(0);
    TempArena temp = temp_arena_begin(arena);
    u32 text_size = vsnprintf(0, 0, format, args) + 1;
    char *text = arena_push(temp.arena, text_size+1, 8);
    vsnprintf(text, text_size, format, args);
    text[text_size] = 0;
    cs_print_text(c, text);
    temp_arena_end(temp);
    va_end(args);
}

#if 1
static i32 found_max_glyph_advance_w(Font *font) {
    i32 result = 0;
    for(u32 i = 0; i < font->glyphs_count; ++i) {
        Glyph *glyph = font->glyphs + i;
        i32 advance_w  = (i32)(glyph->advance_w+0.5f);
        result = max(result, advance_w);
    }
    return result;
}
#endif

static i32 found_max_glyph_h(Font *font) {
    i32 result = 0;
    for(u32 i = 0; i < font->glyphs_count; ++i) {
        Glyph *glyph = font->glyphs + i;
        result = max(result, r2_height(glyph->bounds));
    }
    return result;
}

Console cs_init(struct Font *font, i32 x, i32 y, i32 w, i32 h) {

    Console c = (Console){0};

    c.padding = 4;

    c.font = font;
    c.line = 0;
    c.next_line = 1;
    c.rect = r2_from_wh(x, y, w, h);

    i32 max_advance_w = found_max_glyph_advance_w(font);
    
    c.pixels_per_col = max_advance_w;
    c.pixels_per_row = found_max_glyph_h(font);

    c.visible_cols = (w - c.padding*2) / c.pixels_per_col;
    c.visible_rows = (h - c.padding*2) / c.pixels_per_row;

    assert(c.visible_cols <= MAX_CONSOLE_BUFFER_SIZE);

    c.max_cols = c.visible_cols;
    c.max_rows = MAX_CONSOLE_BUFFER_SIZE / c.visible_cols;

    if(c.visible_rows > c.max_rows) {
        c.visible_rows = c.max_rows;
    }

    return c;
}

static char *cs_get_nullterminated(Arena *arena, Console *c, char *line_buffer) {
    i32 line_size = 0;
    for(i32 i = 0; i <= c->max_cols; ++i) {
        line_size = i;
        if(i < c->max_cols && line_buffer[i] == 0) {
            break;
        }
    }
    char *text = (char *)arena_push(arena, line_size+1, 8);
    memcpy(text, line_buffer, line_size);
    text[line_size] = 0;
    return text;
}

void cs_draw_line(Gpu gpu, Console *c, i32 line, i32 index) {
    Arena *arena = get_scratch_arena(0);
    TempArena temp = temp_arena_begin(arena);

    char *line_buffer = &c->buffer[line * c->max_cols];
    char *text = cs_get_nullterminated(temp.arena, c, line_buffer);
    
    i32 pos_x = c->padding + c->rect.min.x;
    i32 pos_y = -c->padding + c->rect.max.y + -index * c->pixels_per_row - c->pixels_per_row;
    font_draw_text(gpu, c->font, text, (f32)pos_x, (f32)pos_y, v4(1,1,1,1));

    temp_arena_end(temp);
}

void cs_render(Gpu gpu, Console *c) {

    V2 center = r2_center(c->rect);

    gpu_draw_quad_color(gpu, center.x, center.y, (f32)r2_width(c->rect), (f32)r2_height(c->rect), 0, v4(0.0f, 0.0f, 0.0f, 0.4f));

    i32 line = c->next_line - c->visible_rows;
    if(line < 0) {
        line = c->max_rows + line;
    }

    assert(line >= 0);

    if(line < c->next_line) {
        i32 index = 0;
        for(i32 i = line; i < c->next_line; ++i) {
            cs_draw_line(gpu, c, i, index++);
        }

    } else {
        i32 index = 0;
        for(i32 i = line; i < c->max_rows; ++i) {
            cs_draw_line(gpu, c, i, index++);
        }

        for(i32 i = 0; i < c->next_line; ++i) {
            cs_draw_line(gpu, c, i, index++); 
        }
    }

}
