//
// Created by tomas on 24/6/2024.
//

#include "edf_input.h"
#include "edf_platform.h"

V2 input_to_game_coords(V2i in_pos) {
    f32 width  = os_display_width();
    f32 height = os_display_height();
    V2 pos = v2(in_pos.x, in_pos.y);
    pos.x /= width;
    pos.y /= height;
    pos.x -= 0.5f;
    pos.y -= 0.5f;
    pos.x *= width;
    pos.y *= -height;
    return pos;
}

bool point_in_circle(V2 point, V2 c, f32 r) {
    f32 len = v2_len(v2_sub(point, c));
    if(len > r) {
        return false;
    }
    return true;
}

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

