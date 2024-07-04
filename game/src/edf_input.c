//
// Created by tomas on 24/6/2024.
//

#include "edf.h"
#include "edf_input.h"
#include "edf_memory.h"
#include "edf_platform.h"

static inline V2 input_to_game_coords(V2i in_pos) {
    V2 pos = v2((f32)in_pos.x, (f32)in_pos.y);
    //os_print("x: %f, y: %f\n", pos.x, pos.y);
    pos.x -= (r2_width(display) - 1) * 0.5f;
    pos.y -= (r2_height(display) - 1) * 0.5f;
    pos.y *= -1.0f;
    return pos;
}

bool point_in_circle(V2 point, V2 c, f32 r) {
    f32 len = v2_len(v2_sub(point, c));
    if(len > r) {
        return false;
    }
    return true;
}

// ----------------------------------------------
//             Multitouch system
// ----------------------------------------------

void mt_begin(Multitouch *mt, Input *input) {
    mt->input = input;
}

void mt_end(Multitouch *mt, Input *input) {
    memcpy(&mt->last_input, mt->input, sizeof(*mt->input));
}

i32 mt_touch_in_circle(Multitouch *mt, V2 pos, float radii) {
    for(u32 i = 0; i < array_len(mt->input->touches); ++i) {
        Touch *touch = mt->input->touches + i;
        V2 tpos = input_to_game_coords(touch->pos);
        if(touch->down && point_in_circle(tpos, pos, radii)) {
            return i;
        }
    }
    return -1;
}

i32 mt_touch_in_rect(Multitouch *mt, R2 rect) {
    for(u32 i = 0; i < array_len(mt->input->touches); ++i) {
        Touch *touch = mt->input->touches + i;
        V2 tpos = input_to_game_coords(touch->pos);
        if(touch->down && r2_point_overlaps(rect, (i32)tpos.x, (i32)tpos.y)) {
            return i;
        }
    }
    return -1;
}

V2 mt_touch_pos(Multitouch *mt, i32 touch) {
    assert(touch != -1);
    Touch *t = mt->input->touches + touch;
    return input_to_game_coords(t->pos);
}

V2 mt_touch_last_pos(Multitouch *mt, i32 touch) {
    assert(touch != -1);
    Touch *t = mt->last_input.touches + touch;
    return input_to_game_coords(t->pos);
}

b32 mt_touch_down(Multitouch *mt, i32 touch) {
    assert(touch != -1);
    Touch *t = mt->input->touches + touch;
    return t->down;
}

b32 mt_touch_was_down(Multitouch *mt, i32 touch) {
    assert(touch != -1);
    Touch *t = mt->last_input.touches + touch;
    return t->down;
}


// ----------------------------------------------
//             UI system
// ----------------------------------------------

Widget *ui_widget_alloc(Ui *ui, struct Arena *arena) {

    Widget *result = 0;
    
    if(ui->first_free) {
        result = ui->first_free;
        ui->first_free = ui->first_free->next;
        memset(result, 0, sizeof(*result));
    } else {
        result = (Widget *)arena_push(arena, sizeof(*result), 8);
    }
    assert(result);

    result->touch = -1;
    result->widget_tint = v4(1, 1, 1, 1);
    
    if(!ui->widgets) {
        ui->widgets = result;
    } else {
        result->next = ui->widgets;
        result->next->prev = result;
        result->prev = 0;
        ui->widgets = result;
    }

    return result;
}

void ui_widget_free(Ui *ui, Widget *widget) {
    
    if(!widget->prev && !widget->next) {
        ui->widgets = 0;
    }else if(!widget->prev) {
        ui->widgets = widget->next;
        widget->next->prev = 0;
    } else if(!widget->next) {
        widget->prev->next = 0;
    }

    widget->prev = 0;
    widget->next = ui->first_free;
    ui->first_free = widget;
}

Joystick *ui_joystick_alloc(Ui *ui, struct Arena *arena, V2 pos, R2 rect, f32 inner_radii, f32 outer_raddi,
                            Texture inner_texture, Texture outer_texture, V4 tint) {
    Widget *result = ui_widget_alloc(ui, arena);
    result->type = WIDGET_TYPE_JOYSTICK;
    result->pos = pos;
    
    Joystick *joystick = &result->joystick;

    result->joystick.max_distance = outer_raddi - inner_radii*0.5f;;
    result->joystick.outer_radii = outer_raddi;
    result->joystick.inner_radii = inner_radii;
    result->joystick.c_pos = pos;
    result->joystick.saved_pos = pos;
    result->joystick.inner_texture = inner_texture;
    result->joystick.outer_texture = outer_texture;
    result->joystick.rect = rect;

    i32 pos_x = (i32)(pos.x - outer_raddi);
    i32 pos_y = (i32)(pos.y - outer_raddi);

    joystick->widget_rect = r2_from_wh(pos_x, pos_y, (i32)outer_raddi*2, (i32)outer_raddi*2);
    joystick->widget_tint = tint;

    return &result->joystick;
}

Button *ui_button_alloc(Ui *ui, struct Arena *arena, V2 pos, float radii, Texture texture, V4 tint) {
    Widget *result = ui_widget_alloc(ui, arena);
    result->type = WIDGET_TYPE_BUTTON;
    result->pos = pos;

    Button *button = &result->button;
    button->radii = radii;
    button->texture = texture;

    i32 pos_x = (i32)(pos.x - radii);
    i32 pos_y = (i32)(pos.y - radii);
    button->widget_rect = r2_from_wh(pos_x, pos_y, (i32)button->radii*2, (i32)button->radii*2);
    button->widget_tint = tint;
    button->tint = v4(1, 1, 1, 1);

    return button;
}

b32 ui_clean_position(Ui *ui, V2 pos, Widget *me) {

    Widget *widget = ui->widgets;
    while(widget) {
        if(me != widget) {
            if(r2_point_overlaps(widget->widget_rect, pos.x, pos.y)) {
                return false;
            }
        }
        widget = widget->next;
    }

    return true;

}

static inline b32 ui_touch_is_not_been_used(Ui *ui, int touch) {
    Widget *widget = ui->widgets;
    while(widget) {
        if(widget->touch == touch) {
            return false;
        }
        widget = widget->next;
    }
    return true;
}

void ui_begin(Ui *ui, Multitouch *mt, Input *input, f32 dt) {

    mt_begin(mt, input);

    Widget *widget = ui->widgets;
    while(widget) {
        switch (widget->type) {
            case WIDGET_TYPE_BUTTON: {
                Button *button = &widget->button;

                if(button->touch == -1) {
                    i32 touch = mt_touch_in_circle(mt, button->pos, button->radii);
                    if(ui_touch_is_not_been_used(ui, touch)) {
                        button->touch = touch;
                    }
                }

                if(button->touch != -1) {
                    if(mt_touch_down(mt, button->touch)) {
                        button->tint = v4(0.5f, 0.5f, 0.5f, 1);
                    }
                } else {
                    button->tint = v4(1, 1, 1, 1);
                }

            } break;
            case WIDGET_TYPE_JOYSTICK: {
                
                Joystick *joystick = &widget->joystick;

                if(joystick->touch == -1) {
                    
                    joystick->pos = joystick->saved_pos;
                    joystick->c_pos = joystick->pos;
                    
                    joystick->touch = mt_touch_in_rect(mt, joystick->rect);

                    if(joystick->touch != -1) {
                        V2 pos = mt_touch_pos(mt, joystick->touch);
                        if(ui_clean_position(ui, pos, widget)) {
                            joystick->pos = pos;
                            joystick->c_pos = joystick->pos;
                        } else {
                            joystick->touch = -1;
                        }
                    }
                }

                if(joystick->touch != -1) {
                    if(mt_touch_down(mt, joystick->touch)) {
                        i32 uid = mt->input->touches[joystick->touch].uid;
                        i32 last_uid = mt->last_input.touches[joystick->touch].uid;
                        if(uid == last_uid) {
                            joystick->c_pos = mt_touch_pos(mt, joystick->touch);
                        } else {
                            //cs_print(gcs, "current: %d, last: %d\n", uid, last_uid);
                            joystick->touch = -1;
                        }
                    }
                }

                V2 diff = v2_sub(joystick->pos, joystick->c_pos);
                f32 len = v2_len(diff);
                if(len > joystick->max_distance) {
                    V2 dir      = v2_normalized(diff);
                    joystick->pos.x = joystick->c_pos.x + dir.x * joystick->max_distance;
                    joystick->pos.y = joystick->c_pos.y + dir.y * joystick->max_distance;
                }

            } break;
        }

        widget = widget->next;
    }

}

void ui_end(Ui *ui, Multitouch *mt, Input *input) {

    Widget *widget = ui->widgets;
    while(widget) {
        if(widget->touch != -1 && !mt_touch_down(mt, widget->touch)) {
            widget->touch = -1;
        }
        widget = widget->next;
    }

    mt_end(mt, input);
}

void ui_render(Gpu gpu, Ui *ui) {
    Widget *widget = ui->widgets;
    while(widget) {
        switch (widget->type) {
            case WIDGET_TYPE_BUTTON: {
                Button *button = &widget->button;
                gpu_draw_quad_texture_tinted(gpu, button->pos.x, button->pos.y,
                                             button->radii * 2, button->radii * 2,
                                             0, button->texture, v4_had(button->widget_tint, button->tint));

            } break;
            case WIDGET_TYPE_JOYSTICK: {
                Joystick *joystick = &widget->joystick;

                gpu_draw_quad_texture_tinted(gpu, joystick->pos.x, joystick->pos.y,
                                             joystick->outer_radii * 2, joystick->outer_radii * 2,
                                             0, joystick->outer_texture, joystick->widget_tint);

                gpu_draw_quad_texture_tinted(gpu, joystick->c_pos.x, joystick->c_pos.y,
                                             joystick->inner_radii * 2, joystick->inner_radii * 2,
                                             0, joystick->inner_texture, joystick->widget_tint);

            } break;
        }
        widget = widget->next;
    }

}

b32 ui_widget_is_active_(Multitouch *mt, Widget *widget) {
    if(widget->touch == -1) return false;
    return mt_touch_down(mt, widget->touch);
}

b32 ui_widget_just_press_(Multitouch *mt, Widget *widget) {
    if(widget->touch == -1) return false;
    b32 is_down = mt_touch_down(mt, widget->touch);
    b32 was_down = mt_touch_was_down(mt, widget->touch);
    return is_down && !was_down;
}

b32 ui_widget_just_up_(Multitouch *mt, Widget *widget) {
    if(widget->touch == -1) return false;
    b32 is_down = mt_touch_down(mt, widget->touch);
    b32 was_down = mt_touch_was_down(mt, widget->touch);
    b32 result = !is_down && was_down;
    return result;
}

b32 ui_button_just_up(Multitouch *mt, Button *button) {
    if(button->touch == -1) return false;
    if(ui_widget_just_up(mt, button)) {
        V2 pos = mt_touch_last_pos(mt, button->touch);
        if(r2_point_overlaps(button->widget_rect, pos.x, pos.y)) {
            return true;
        }
    }
    return false;
}
