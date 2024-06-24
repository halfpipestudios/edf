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

void mt_update(Multitouch *mt, Input *input) {

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

    memcpy(&mt->last_input, mt->input, sizeof(*mt->input));

}

b32 mt_touch_in_circle(Multitouch *mt, i32 *touch, V2 pos, float radii) {
    assert(*touch == -1);
    for(u32 i = 0; i < mt->input->count; ++i) {
        Touch *t = mt->input->touches + mt->input->locations[i];
        V2 tpos = input_to_game_coords(t->pos);
        if(!mt->registry[mt->input->locations[i]] && point_in_circle(tpos, pos, radii)) {
            mt->registry[mt->input->locations[i]] = touch;
            *touch = mt->input->locations[i];
            return true;
        }
    }
    return false;
}

b32 mt_touch_in_rect(Multitouch *mt, i32 *touch, R2 rect) {
    assert(*touch == -1);
    for(u32 i = 0; i < mt->input->count; ++i) {
        Touch *t = mt->input->touches + mt->input->locations[i];
        V2 tpos = input_to_game_coords(t->pos);
        if(!mt->registry[mt->input->locations[i]] && r2_point_overlaps(rect, tpos.x, tpos.y)) {
            mt->registry[mt->input->locations[i]] = touch;
            *touch = mt->input->locations[i];
            return true;
        }
    }
    return false;
}

V2 mt_touch_pos(Multitouch *mt, int touch) {
    assert(touch != -1);
    Touch *t = mt->input->touches + mt->input->locations[touch];
    return input_to_game_coords(t->pos);
}

