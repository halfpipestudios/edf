//
// Created by tomas on 24/6/2024.
//

#include "edf.h"
#include "edf_input.h"
#include "edf_memory.h"
#include "edf_platform.h"

V2 input_to_game_coords(V2i in_pos) {
    R2 device = os_device_rect();
    R2 display = os_display_rect();
    V2 pos = v2(in_pos.x, in_pos.y);
    pos.x -= display.min.x;
    pos.y -= display.min.y;
    pos.x /= r2_width(display);
    pos.y /= r2_height(display);
    pos.x -= 0.5f;
    pos.y -= 0.5f;
    pos.x *= VIRTUAL_RES_X;
    pos.y *= -VIRTUAL_RES_Y;
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
    for(u32 i = 0; i < mt->last_input.count; ++i) {
        b32 found = false;
        int location_to_found = mt->last_input.locations[i];
        for(u32 j = 0; j < mt->input->count; ++j) {
            if(location_to_found == mt->input->locations[j]) {
                found = true;
                break;
            }
        }

        if(!found) {
            if(mt->registry[location_to_found]) {
                *(mt->registry[location_to_found]) = -1;
            }
            mt->registry[location_to_found] = 0;
        }
    }
}

void mt_end(Multitouch *mt, Input *input) {
    memcpy(&mt->last_input, mt->input, sizeof(*mt->input));
}

static inline b32 found_location_in_last_input(Multitouch *mt, int location_index) {
    for(u32 j = 0; j < mt->last_input.count; ++j) {
        if(location_index == mt->last_input.locations[j]) {
            return true;
            break;
        }
    }
    return false;
}

b32 mt_touch_in_circle(Multitouch *mt, i32 *touch, V2 pos, float radii) {
    if(*touch != -1) return false;
    for(u32 i = 0; i < mt->input->count; ++i) {
        Touch *t = mt->input->touches + mt->input->locations[i];
        V2 tpos = input_to_game_coords(t->pos);
        int location_index = mt->input->locations[i];
        if(!mt->registry[location_index] && point_in_circle(tpos, pos, radii)) {
            mt->registry[location_index] = touch;
            *touch = location_index;
            return true;
        }
    }
    return false;
}

b32 mt_touch_just_in_circle(Multitouch *mt, i32 *touch, V2 pos, float radii) {
    if(*touch != -1) return false;
    for(u32 i = 0; i < mt->input->count; ++i) {
        
        int location_index = mt->input->locations[i];
        b32 found = found_location_in_last_input(mt, location_index);

        if(!found) {
            Touch *t = mt->input->touches + mt->input->locations[i];
            V2 tpos = input_to_game_coords(t->pos);
            if(!mt->registry[location_index] && point_in_circle(tpos, pos, radii)) {
                mt->registry[location_index] = touch;
                *touch = location_index;
                return true;
            }
        }
    }
    return false;
}

b32 mt_touch_in_rect(Multitouch *mt, i32 *touch, R2 rect) {
    if(*touch != -1) return false;
    for(u32 i = 0; i < mt->input->count; ++i) {
        Touch *t = mt->input->touches + mt->input->locations[i];
        V2 tpos = input_to_game_coords(t->pos);
        int location_index = mt->input->locations[i];
        if(!mt->registry[location_index] && r2_point_overlaps(rect, (i32)tpos.x, (i32)tpos.y)) {
            mt->registry[location_index] = touch;
            *touch = location_index;
            return true;
        }
    }
    return false;
}

b32 mt_touch_just_in_rect(Multitouch *mt, i32 *touch, R2 rect) {
    if(*touch != -1) return false;
    for(u32 i = 0; i < mt->input->count; ++i) {

        int location_index = mt->input->locations[i];
        b32 found = found_location_in_last_input(mt, location_index);

        if(!found) {
            Touch *t = mt->input->touches + mt->input->locations[i];
            V2 tpos = input_to_game_coords(t->pos);
            if(!mt->registry[location_index] && r2_point_overlaps(rect, (i32)tpos.x, (i32)tpos.y)) {
                mt->registry[location_index] = touch;
                *touch = location_index;
                return true;
            }
        }
    }
    return false;
}

V2 mt_touch_pos(Multitouch *mt, int touch) {
    assert(touch != -1);
    Touch *t = mt->input->touches + touch;
    return input_to_game_coords(t->pos);
}

b32 mt_touch_down(Multitouch *mt, int touch) {
    return touch != -1;
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

b32 ui_widget_is_active_(Multitouch *mt, Widget *widget) {
    return mt_touch_down(mt, widget->touch);
}

Joystick *ui_joystick_alloc(Ui *ui, struct Arena *arena, V2 pos, R2 rect, 
                            f32 inner_radii, f32 outer_raddi, Texture inner_texture, Texture outer_texture) {
    Widget *result = ui_widget_alloc(ui, arena);
    result->type = WIDGET_TYPE_JOYSTICK;
    result->pos = pos;
    
    result->joystick.max_distance = outer_raddi - inner_radii*0.5f;;
    result->joystick.outer_radii = outer_raddi;
    result->joystick.inner_radii = inner_radii;
    result->joystick.c_pos = pos;
    result->joystick.saved_pos = pos;
    result->joystick.inner_texture = inner_texture;
    result->joystick.outer_texture = outer_texture;
    result->joystick.rect = rect;

    return &result->joystick;
}

Button *ui_button_alloc(Ui *ui, struct Arena *arena, V2 pos, float radii, Texture texture) {
    Widget *result = ui_widget_alloc(ui, arena);
    result->type = WIDGET_TYPE_BUTTON;
    result->pos = pos;
    
    result->button.radii = radii;
    result->button.texture = texture;
    
    return &result->button;
}

void ui_update(Ui *ui, Multitouch *mt, f32 dt) {
    Widget *widget = ui->widgets;
    while(widget) {

        switch (widget->type) {
            case WIDGET_TYPE_BUTTON: {
                Button *button = &widget->button;
                mt_touch_just_in_circle(mt, &button->touch, button->pos, button->radii);
                if(mt_touch_down(mt, button->touch)) {
                    button->tint = v4(0.5f, 0.5f, 0.5f, 1);
                } else {
                    button->tint = v4(1, 1, 1, 1);
                }

            } break;
            case WIDGET_TYPE_JOYSTICK: {
                
                Joystick *joystick = &widget->joystick;

                if(mt_touch_in_rect(mt, &joystick->touch, joystick->rect)) {
                    joystick->pos = mt_touch_pos(mt, joystick->touch);
                    joystick->c_pos = joystick->pos;
                } 
                
                if(mt_touch_down(mt, joystick->touch)) {
                    joystick->c_pos = mt_touch_pos(mt, joystick->touch);
                } else {
                    joystick->pos = joystick->saved_pos;
                    joystick->c_pos = joystick->pos;
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

void ui_render(Gpu gpu, Ui *ui) {

    Widget *widget = ui->widgets;
    while(widget) {

        switch (widget->type) {
            case WIDGET_TYPE_BUTTON: {
                Button *button = &widget->button;

                gpu_draw_quad_texture_tinted(gpu, button->pos.x, button->pos.y,
                                             button->radii * 2, button->radii * 2,
                                             0, button->texture, button->tint);

            } break;
            case WIDGET_TYPE_JOYSTICK: {

                Joystick *joystick = &widget->joystick;

                gpu_draw_quad_texture(gpu, joystick->pos.x, joystick->pos.y, 
                                      joystick->outer_radii * 2, joystick->outer_radii * 2,
                                      0, joystick->outer_texture);

                gpu_draw_quad_texture(gpu, joystick->c_pos.x, joystick->c_pos.y, 
                                      joystick->inner_radii * 2, joystick->inner_radii * 2,
                                      0, joystick->inner_texture);

            } break;
        }

        widget = widget->next;
    }

}
