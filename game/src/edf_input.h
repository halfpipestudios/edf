//
// Created by tomas on 24/6/2024.
//

#ifndef EDF_EDF_INPUT_H
#define EDF_EDF_INPUT_H

#include "edf_common.h"
#include "edf_math.h"

typedef struct Touch {
    V2i pos;
    u64 uid;
    i32 location;
} Touch;

#define MAX_TOUCHES 5

typedef struct Input {
    Touch touches[MAX_TOUCHES];
    i32 locations[MAX_TOUCHES];
    u32 count;
} Input;

typedef struct Multitouch {
    Input *input;
    Input last_input;
    int *registry[MAX_TOUCHES];
} Multitouch;

void mt_begin(Multitouch *mt, Input *input);
void mt_end(Multitouch *mt, Input *input);

b32 mt_touch_in_circle(Multitouch *mt, i32 *touch, V2 pos, float radii);
b32 mt_touch_just_in_circle(Multitouch *mt, i32 *touch, V2 pos, float radii);
b32 mt_touch_in_rect(Multitouch *mt, i32 *touch, R2 rect);
b32 mt_touch_just_in_rect(Multitouch *mt, i32 *touch, R2 rect);
V2 mt_touch_pos(Multitouch *mt, int touch);
b32 mt_touch_down(Multitouch *mt, int touch);

#endif //EDF_EDF_INPUT_H