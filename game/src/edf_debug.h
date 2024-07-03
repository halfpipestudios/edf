#ifndef EDF_DEBUG_H
#define EDF_DEBUG_H

#include "edf_common.h"
#include "edf_platform.h"

// -------------------------------------------------
// Console
// -------------------------------------------------

#define MAX_CONSOLE_BUFFER_SIZE 1024*4

typedef struct Console {
    
    char buffer[MAX_CONSOLE_BUFFER_SIZE];
    i32 max_cols;
    i32 max_rows;

    i32 col;
    i32 line;
    i32 next_line;
    
    struct Font *font;
    R2 rect;
    i32 pixels_per_col;
    i32 pixels_per_row;
    
    i32 visible_cols;
    i32 visible_rows;

    i32 padding;
} Console;

Console cs_init(struct Font *font, i32 x, i32 y, i32 w, i32 h);
void cs_print(Console *c, char *format, ...);
void cs_render(Gpu gpu, Console *c);

extern Console *gcs;

// -------------------------------------------------
// Arena Viewer
// -------------------------------------------------

typedef struct ArenaView {
    char *name;
    struct Arena *arena;

    struct ArenaView *next;
} ArenaView;

typedef struct ArenaViewer {
    struct Arena *arena;
    i32 x, y, w;
    R2 rect;
    ArenaView *views;
    u32 views_count;
    i32 padding_t_b;
    i32 padding_l_r;
    i32 view_h;
    struct Font *font;
} ArenaViewer;

ArenaViewer av_init(struct Arena *arena, struct Font *font, i32 x, i32 y, i32 w);
void av_add_arena(ArenaViewer *av, struct Arena *arena, char *name);
void av_render(Gpu gpu, ArenaViewer *av);

#endif // EDF_DEBUG_H
