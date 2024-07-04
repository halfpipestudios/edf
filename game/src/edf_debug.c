#include "edf_platform.h"
#include "edf_debug.h"
#include "edf_memory.h"
#include "edf_font.h"

#include <stdarg.h>
#include <stdio.h>

// -------------------------------------------------
// Console
// -------------------------------------------------

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
static f32 found_max_glyph_advance_w(Font *font) {
    f32 result = 0;
    for(u32 i = 0; i < font->glyphs_count; ++i) {
        Glyph *glyph = font->glyphs + i;
        f32 advance_w  = glyph->advance_w;
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
    c.rect = r2_from_wh(x, y-h, w, h);

    f32 max_advance_w = found_max_glyph_advance_w(font);
    
    
    c.pixels_per_row = found_max_glyph_h(font);

    c.visible_cols = ((f32)(w - c.padding*2) / max_advance_w + 0.5f);
    c.visible_rows = (h - c.padding*2) / c.pixels_per_row;

    c.pixels_per_col = (w - c.padding*2) / c.visible_cols;

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
    
    R2 text_size = font_size_text(c->font, text);
    if(!r2_invalid(text_size)) {
        i32 pos_x = c->padding + c->rect.min.x + text_size.min.x;
        i32 pos_y = -c->padding + c->rect.max.y + -index * c->pixels_per_row - c->pixels_per_row;
        font_draw_text(gpu, c->font, text, (f32) pos_x, (f32) pos_y, v4(1, 1, 1, 1));
    }

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

// -------------------------------------------------
// Arena Viewer
// -------------------------------------------------

ArenaViewer av_init(struct Arena *arena, struct Font *font, i32 x, i32 y, i32 w) {
    ArenaViewer av = {0};
    av.x = x;
    av.y = y;
    av.w = w;
    av.rect = r2_set_invalid();
    av.arena = arena;
    av.padding_t_b = 50;
    av.padding_l_r = 20;
    av.view_h = 40;
    av.font = font;
    return av;
}

void av_add_arena(ArenaViewer *av, struct Arena *arena, char *name) {
    ArenaView *view = (ArenaView *)arena_push(av->arena, sizeof(*view), 8);
    view->arena = arena;
    view->name = name;
    view->next = av->views;
    av->views = view;
    av->views_count++;
}

static inline char *calculate_used_text(Arena *arena) {
    static char buffer[1024];
    
    f32 used = arena->used;
    char *used_text = 0;

    f32 size = arena->size;
    char *size_text = 0;
    
    
    if(used >= gb(1)) {
        used = (f32)used / (f32)gb(1);
        used_text = "GB";
    } else if(used >= mb(1)) {
        used = (f32)used / (f32)mb(1);
        used_text = "MB";
    } else if(used >= kb(1)){
        used = (f32)used / (f32)kb(1);
        used_text = "KB";
    } else {
        used_text = "B";
    }

    if(size >= gb(1)) {
        size = (f32)size / (f32)gb(1);
        size_text = "GB";
    } else if(size >= mb(1)) {
        size = (f32)size / (f32)mb(1);
        size_text = "MB";
    } else if(size >= kb(1)){
        size = (f32)size / (f32)kb(1);
        size_text = "KB";
    } else {
        size_text = "B";
    }

    f32 used_decimals = used - floorf(used);
    f32 size_decimals = size - floorf(size);

    if(used_decimals >= 0.1f && size_decimals >= 1.0f) {
        sprintf(buffer, "used: %.2f%s | size: %.2f%s", used, used_text, size, size_text);
    } else if (used_decimals < 0.1f && size_decimals < 1.0f) {
        sprintf(buffer, "used: %d%s | size: %d%s", (i32)used, used_text, (i32)size, size_text);
    } else if(used_decimals >= 0.1f) {
        sprintf(buffer, "used: %.2f%s | size: %d%s", used, used_text, (i32)size, size_text);
    } else {
        sprintf(buffer, "used: %d%s | size: %.2f%s", (i32)used, used_text, size, size_text);    
    }

    return buffer;
}

void av_render(Gpu gpu, ArenaViewer *av) {
    
    // NOTE: calculate viewer height
    i32 rect_h = av->views_count*(av->view_h+av->padding_t_b) + av->padding_t_b;
    i32 y = av->y - rect_h;
    av->rect = r2_from_wh(av->x, y, av->w, rect_h);
    
    V2 center = r2_center(av->rect);
    i32 view_h = av->view_h;
    i32 view_hh = av->view_h/2;
    i32 view_w = r2_width(av->rect)-av->padding_l_r*2;

    gpu_draw_quad_color(gpu, center.x, center.y, r2_width(av->rect), r2_height(av->rect), 0, v4(0, 0, 0, 0.4f));
    i32 base_y = av->rect.max.y - av->padding_t_b;

    u32 index = 0;
    ArenaView *view = av->views;
    while(view) {
        
        i32 pos_y = base_y - (view_h + av->padding_t_b)*index - view_hh;

        R2 name_size = font_size_text(av->font, view->name);
        i32 name_pos_y = pos_y+view_hh-name_size.min.y;
        i32 name_pos_x = av->rect.min.x+av->padding_l_r;
        
        font_draw_text(gpu, av->font, view->name, name_pos_x, name_pos_y, v4(1, 1, 1, 1));

        gpu_draw_quad_color(gpu, center.x, pos_y,
                            view_w, view_h, 0, v4(1, 0, 0, 0.4f));
        
        f32 ratio = (f32)view->arena->used / (f32)view->arena->size;
        i32 used_w = (i32)((f32)view_w * ratio) + ratio;

        i32 used_pos_x = av->rect.min.x + av->padding_l_r + used_w/2;
        gpu_draw_quad_color(gpu, used_pos_x, pos_y,
                            used_w, view_h, 0, v4(0, 1, 0, 0.4f));
        
        char *used_text = calculate_used_text(view->arena);
        R2 use_size = font_size_text(av->font, used_text);
        i32 use_pos_y = (pos_y - view_hh) + (view_hh - r2_height(use_size)/2);

        font_draw_text(gpu, av->font, used_text, av->rect.min.x + av->padding_l_r+use_size.min.x, 
                       use_pos_y-use_size.min.y, v4(1, 1, 1, 0.4));

        ++index;
        view = view->next;
    }
}
