#ifndef EDF_DEBUG_H
#define EDF_DEBUG_H

#include "edf_common.h"
#include "edf_platform.h"

#define MAX_CONSOLE_BUFFER_SIZE 1024*4

typedef struct Console {
    char buffer[MAX_CONSOLE_BUFFER_SIZE];
    u32 max_cols;
    u32 max_lines;

    u32 col;
    u32 lines_start;
    u32 line;

    struct Font *font;
    u32 w;
    u32 h;

} Console;

Console cs_init(struct Font *font, u32 w, u32 h);
void cs_print(Console *c, char *text);
void cs_render(Gpu gpu, Console *c);

#endif // EDF_DEBUG_H
