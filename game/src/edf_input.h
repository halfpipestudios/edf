//
// Created by tomas on 24/6/2024.
//

#ifndef EDF_EDF_INPUT_H
#define EDF_EDF_INPUT_H

#include "edf_common.h"
#include "edf_math.h"

struct Arena;
typedef void * Texture;
typedef void * Gpu;

// ----------------------------------------------
//             Multitouch system
// ----------------------------------------------

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

// ----------------------------------------------
//             UI system
// ----------------------------------------------

typedef enum WidgetType {
    WIDGET_TYPE_JOYSTICK,
    WIDGET_TYPE_BUTTON,
} WidgetType;

#define WIDGET_HEADER   \
struct {                \
    WidgetType type;    \
    V2 pos;             \
    i32 touch;          \
    union Widget *next; \
    union Widget *prev; \
};                      

typedef struct Joystick {
    WIDGET_HEADER

    V2 c_pos;
    V2 saved_pos;
    f32 max_distance;
    f32 inner_radii;
    f32 outer_radii;

    R2 rect;

    Texture inner_texture;
    Texture outer_texture;
} Joystick;

typedef struct Button {
    WIDGET_HEADER

    f32 radii;
    V3 tint;
    Texture texture;
} Button;

typedef union Widget {
    WIDGET_HEADER
    Joystick joystick;
    Button button;
} Widget;

typedef struct Ui {
    Widget *widgets;
    Widget *first_free;
} Ui;

void ui_update(Ui *ui, Multitouch *mt, f32 dt);
void ui_render(Gpu gpu, Ui *ui);

Widget *ui_widget_alloc(Ui *ui, struct Arena *arena);
void ui_widget_free(Ui *ui, Widget *widget);

#define ui_widget_is_active(mt, widget) ui_widget_is_active_((mt), (Widget *)(widget))
b32 ui_widget_is_active_(Multitouch *mt, Widget *widget);

Joystick *ui_joystick_alloc(Ui *ui, struct Arena *arena, V2 pos, R2 rect, 
                            f32 inner_radii, f32 outer_raddi, Texture inner_texture, Texture outer_texture);
Button *ui_button_alloc(Ui *ui, struct Arena *arena, V2 pos, float radii, Texture texture);

#endif //EDF_EDF_INPUT_H
