//
// Created by tomas on 24/6/2024.
//

#ifndef EDF_INPUT_H
#define EDF_INPUT_H

#include "edf_common.h"
#include "edf_math.h"

struct Arena;
typedef void * Texture;
typedef void * Gpu;

// ----------------------------------------------
//             Multitouch system
// ----------------------------------------------

typedef struct Touch {
    u64 uid;
    u64 hash;

    V2i pos;
    b32 down;
    b32 up;
} Touch;


#define MAX_TOUCHES 5
typedef struct Input {
    Touch touches[MAX_TOUCHES];
    i32 count;
} Input;

#define TOUCH_INVALID_UID 0
typedef struct Multitouch {
    Input *input;
    Input last_input;
} Multitouch;

void mt_begin(Multitouch *mt, Input *input);
void mt_end(Multitouch *mt, Input *input);

i32 mt_touch_in_circle(Multitouch *mt, V2 pos, float radii);
i32 mt_touch_in_rect(Multitouch *mt, R2 rect);

V2 mt_touch_pos(Multitouch *mt, i32 touch);
V2 mt_touch_last_pos(Multitouch *mt, i32 touch);
b32 mt_touch_down(Multitouch *mt, i32 touch);
b32 mt_touch_was_down(Multitouch *mt, i32 touch);

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
    V4 widget_tint;     \
    R2 widget_rect;     \
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
    V4 tint;
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

void ui_begin(Ui *ui, Multitouch *mt, Input *input, f32 dt);
void ui_end(Ui *ui, Multitouch *mt, Input *input);

void ui_render(Gpu gpu, Ui *ui);

b32 ui_clean_position(Ui *ui, V2 pos, Widget *me);

Widget *ui_widget_alloc(Ui *ui, struct Arena *arena);
void ui_widget_free(Ui *ui, Widget *dirty);

#define ui_widget_is_active(mt, widget) ui_widget_is_active_((mt), (Widget *)(widget))
b32 ui_widget_is_active_(Multitouch *mt, Widget *widget);
#define ui_widget_just_press(mt, widget) ui_widget_just_press_((mt), (Widget *)(widget))
b32 ui_widget_just_press_(Multitouch *mt, Widget *widget);
#define ui_widget_just_up(mt, widget) ui_widget_just_up_((mt), (Widget *)(widget))
b32 ui_widget_just_up_(Multitouch *mt, Widget *widget);

Button *ui_button_alloc(Ui *ui, struct Arena *arena, V2 pos, float radii, Texture texture, V4 tint);
b32 ui_button_just_up(Multitouch *mt, Button *button);

Joystick *ui_joystick_alloc(Ui *ui, struct Arena *arena, V2 pos, R2 rect, f32 inner_radii, f32 outer_raddi,
                            Texture inner_texture, Texture outer_texture, V4 tint);



#endif //EDF_INPUT_H
