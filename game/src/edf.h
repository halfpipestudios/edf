#ifndef EDF_H
#define EDF_H

#include "edf_platform.h"

struct Memory;

void game_init(struct Memory *memory);
void game_update(struct Memory *memory, Input *input, f32 dt);
void game_render(struct Memory *memory);
void game_shutdown(struct Memory *memory);

#endif // EDF_H
