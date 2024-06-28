#include "edf_platform.h"
#include "edf_debug.h"
#include "edf_memory.h"
#include "edf_font.h"

void cs_print(Console *c, char *text) {
    u32 len = strlen(text);

    for(u32 i = 0; i < len; ++i) {
        
        char character = text[i];
        if(character == '\n') {
            c->col = 0;
            c->line = (c->line + 1) % c->max_lines;

            if(c->line == c->lines_start) {
                memset(&c->buffer[c->line * c->max_cols], 0, c->max_cols);
                c->lines_start = (c->lines_start + 1) % c->max_lines;
            }

            continue;
        }

        if(c->col >= c->max_cols) {
            c->col = 0;
            c->line = (c->line + 1) % c->max_lines;

            if(c->line == c->lines_start) {
                memset(&c->buffer[c->line * c->max_cols], 0, c->max_cols);
                c->lines_start = (c->lines_start + 1) % c->max_lines;
            }

        }



        c->buffer[c->line * c->max_cols + c->col++] = character;
    }
}

Console cs_init(struct Font *font, u32 w, u32 h) {

    u32 num_cols = w / font->size*2;
    u32 num_lines = h / font->size;

    Console c = {0};
    c.w = w;
    c.h = h;
    c.max_lines = MAX_CONSOLE_BUFFER_SIZE / num_cols;
    c.max_cols = num_cols;
    c.font = font;

    return c;
}

static char *cs_get_nullterminated(Arena *arena, Console *c, char *line_buffer) {
    u32 line_size = 0;
    for(u32 i = 0; i <= c->max_cols; ++i) {
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

void cs_render(Gpu gpu, Console *c) {

    i32 offset_x = -VIRTUAL_RES_X*0.5f;
    i32 offset_y = -VIRTUAL_RES_Y*0.5f + c->h;

    gpu_draw_quad_color(gpu, offset_x+c->w*0.5f, offset_y-c->h*0.5f, c->w+20, c->h+20, 0, v4(0.5f, 0.5f, 0.5f, 0.5f));

    u32 num_lines = c->h / c->font->size;

    u32 last_line = (c->line + 1) % c->max_lines;
    u32 line = 0;

    if(last_line > num_lines) {
        line = last_line - num_lines;
    } else {
        line = c->max_lines - (num_lines - last_line);
    }

    i32 index = 0;
    do {
        Arena *arena = get_scratch_arena(0);
        TempArena temp = temp_arena_begin(arena);

        char *line_buffer = &c->buffer[line * c->max_cols];
        char *text = cs_get_nullterminated(temp.arena, c, line_buffer);
        

        i32 pos_x = offset_x + 20;
        i32 pos_y = offset_y + -index * c->font->size - c->font->size;
        font_draw_text(gpu, c->font, text, pos_x, pos_y, v4(1,1,1,1));

        temp_arena_end(temp);
        ++index;
        line = (line + 1) % c->max_lines;
    } while(line != last_line);
}
