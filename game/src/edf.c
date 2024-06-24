#include "edf.h"

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

void game_init(Memory *memory) {
    game_state_init(memory);
    GameState *gs = game_state(memory);

    init_scratch_arenas(memory, 3, mb(10));

    gs->platform_arena = arena_create(memory, mb(20));
    gs->game_arena     = arena_create(memory, mb(50));

    gs->gpu   = gpu_load(&gs->platform_arena);
    gs->spu   = spu_load(&gs->platform_arena);
    gs->arial = font_load(gs->gpu, &gs->platform_arena, "arial.ttf", 128);
    gs->times = font_load(gs->gpu, &gs->platform_arena, "times.ttf", 64);

    gs->ship_bitmap       = bitmap_load(&gs->game_arena, "Player.png");
    gs->move_outer_bitmap = bitmap_load(&gs->game_arena, "move_outer.png");
    gs->move_inner_bitmap = bitmap_load(&gs->game_arena, "move_inner.png");
    gs->boost_bitmap      = bitmap_load(&gs->game_arena, "boost.png");

    gs->ship_texture       = gpu_texture_load(gs->gpu, &gs->ship_bitmap);
    gs->move_outer_texture = gpu_texture_load(gs->gpu, &gs->move_outer_bitmap);
    gs->move_inner_texture = gpu_texture_load(gs->gpu, &gs->move_inner_bitmap);
    gs->boost_texture      = gpu_texture_load(gs->gpu, &gs->boost_bitmap);

    f32 size = 32.0f * 3.0f;
    gs->ship =
        sprite_load(&gs->game_arena, v2(0, 0), v2(size, size), v3(1, 1, 1), 0, gs->ship_texture);
    gs->ship_vel = v2(0, 0);
    gs->ship_acc = v2(0, 0);
    gs->ship_damping = 0.4f;

    gs->s_pos = v2(-740, -250);
    gs->c_pos = gs->s_pos;
    gs->joystick_scale   = 4;
    gs->joystick_is_down = false;

    gs->s_inner      = 1.5f;
    gs->button_radii = (gs->boost_bitmap.w * gs->joystick_scale * gs->s_inner) * 0.5f;

    gs->button_is_down[0] = false;
    gs->button_is_down[1] = false;

    gs->button_center  = v2(740, -250);
    gs->button_radii   = 100;

    gs->boost_tint = v3(1, 1, 1);

    gs->joystick_location = -1;
    gs->button_location = -1;

}

void game_update(Memory *memory, Input *input, f32 dt) {
    GameState *gs             = game_state(memory);
    gs->joystick_max_distance = ((f32)gs->move_outer_bitmap.w * 0.5f) * gs->joystick_scale;

    bool joystick_found = false;
    bool button_found = false;
    for(u32 i = 0; i < input->count; ++i) {
        Touch *touch = input->touches + input->locations[i];
        if(touch->event == TOUCH_EVENT_DOWN || touch->event == TOUCH_EVENT_MOVE) {
            V2 pos = input_to_game_coords(touch->pos);
            if(gs->button_location == -1 && point_in_circle(pos, gs->button_center, gs->button_radii)) {
                gs->button_location = input->locations[i];
                gs->button_is_down[0] = true;
            }
            else if(gs->joystick_location == -1 && !point_in_circle(pos, gs->button_center, gs->button_radii)) {
                gs->joystick_location = input->locations[i];
                gs->joystick_is_down = true;
                gs->s_pos = pos;
                gs->c_pos   = gs->s_pos;
            }
        }

        if(gs->joystick_location == input->locations[i]) {
            joystick_found = true;
        }
        if(gs->button_location == input->locations[i]) {
            button_found = true;
        }
    }
    if(joystick_found == false) {
        gs->joystick_location = -1;
        gs->joystick_is_down = false;
        gs->c_pos = gs->s_pos;
    }
    if(button_found == false) {
        gs->button_location = -1;
        gs->button_is_down[0] = false;
    }

    if(gs->joystick_is_down) {
        Touch *touch = input->touches + gs->joystick_location;
        V2 pos = input_to_game_coords(touch->pos);
        if(touch->event == TOUCH_EVENT_DOWN || touch->event == TOUCH_EVENT_MOVE) {
            gs->c_pos = pos;
        }
    }

    V2 diff = v2_sub(gs->s_pos, gs->c_pos);
    f32 len = v2_len(diff);
    if(len > gs->joystick_max_distance) {
        V2 dir      = v2_normalized(diff);
        gs->s_pos.x = gs->c_pos.x + dir.x * gs->joystick_max_distance;
        gs->s_pos.y = gs->c_pos.y + dir.y * gs->joystick_max_distance;
    }

    if(gs->joystick_is_down) {
        if(len > (gs->joystick_max_distance * 0.2f)) {
            V2 dir          = v2_normalized(diff);
            gs->ship->angle = atan2f(dir.y, dir.x) + (PI / 2.0f);
        }
    }

    V2 dir = v2_sub(gs->c_pos, gs->s_pos);
    gs->boost_tint = v3(1, 1, 1);
    if(gs->button_is_down[0]) {
        gs->boost_tint = v3(0.5f, 0.5f, 0.5f);
        V2 dir = {0};
        dir.x = cosf(gs->ship->angle + (PI / 2.0f));
        dir.y = sinf(gs->ship->angle + (PI / 2.0f));
        if(v2_len(dir) > 0) {
            dir = v2_normalized(dir);
            gs->ship_acc.x += dir.x * 800.0f;
            gs->ship_acc.y += dir.y * 800.0f;
        }

    }

    gs->ship->pos.x += gs->ship_vel.x * dt;
    gs->ship->pos.y += gs->ship_vel.y * dt;

    gs->ship_vel.x += gs->ship_acc.x * dt;
    gs->ship_vel.y += gs->ship_acc.y * dt;

    gs->ship_vel.x *= powf(gs->ship_damping, dt);
    gs->ship_vel.y *= powf(gs->ship_damping, dt);

    gs->ship_acc = v2(0, 0);

    gs->button_is_down[1] = gs->button_is_down[0];
}

void game_render(Memory *memory) {
    GameState *gs = game_state(memory);

    u32 w = os_display_width();
    u32 h = os_display_height();

    gpu_frame_begin(gs->gpu);

    gpu_draw_quad_color(gs->gpu, 0, 0, w, h, 0, v3(0.05f, 0.05f, 0.1f));

    sprite_draw(gs->gpu, gs->ship);

    f32 s       = gs->joystick_scale;
    f32 s_inner = gs->s_inner;

    gpu_draw_quad_texture(gs->gpu, gs->s_pos.x, gs->s_pos.y, gs->move_outer_bitmap.w * s,
                          gs->move_outer_bitmap.h * s, 0, gs->move_outer_texture);
    gpu_draw_quad_texture(gs->gpu, gs->c_pos.x, gs->c_pos.y, gs->move_inner_bitmap.w * s * s_inner,
                          gs->move_inner_bitmap.h * s * s_inner, 0, gs->move_inner_texture);
    gpu_draw_quad_texture_tinted(gs->gpu, gs->button_center.x, gs->button_center.y, gs->button_radii * 2,
                          gs->button_radii * 2, 0, gs->boost_texture, gs->boost_tint);

    gpu_frame_end(gs->gpu);
}

void game_shutdown(Memory *memory) {
    GameState *gs = game_state(memory);
    gpu_unload(gs->gpu);
}

void game_resize(Memory *memory, u32 w, u32 h) {
    GameState *gs = game_state(memory);
    gpu_resize(gs->gpu, w, h);
}

/*
void game_touches_down(struct Memory *memory, struct Input *input) {
    if(input->touches_count > 2) {
        return;
    }

    GameState *gs = game_state(memory);

    for(u32 i = 0; i < input->touches_count; i++) {

        os_print("pointer down index: %d\n", input->touches[i].index);

        f32 width  = os_display_width();
        f32 height = os_display_height();
        V2 pos = v2(input->touches[i].pos.x, input->touches[i].pos.y);
        pos.x /= width;
        pos.y /= height;
        pos.x -= 0.5f;
        pos.y -= 0.5f;
        pos.x *= width;
        pos.y *= -height;
        if(point_in_circle(pos, gs->button_center, gs->button_radii)) {
            gs->button_is_down[0] = true;
        } else {
            gs->s_pos.x = input->touches[i].pos.x;
            gs->s_pos.y = input->touches[i].pos.y;
            gs->c_pos   = gs->s_pos;

            gs->s_pos.x /= width;
            gs->s_pos.y /= height;
            gs->s_pos.x -= 0.5f;
            gs->s_pos.y -= 0.5f;
            gs->s_pos.x *= width;
            gs->s_pos.y *= -height;

            gs->c_pos.x /= width;
            gs->c_pos.y /= height;
            gs->c_pos.x -= 0.5f;
            gs->c_pos.y -= 0.5f;
            gs->c_pos.x *= width;
            gs->c_pos.y *= -height;
            gs->joystick_is_down = true;
        }
    }
}

void game_touches_up(struct Memory *memory, struct Input *input) {
    if(input->touches_count > 2) {
        return;
    }

    GameState *gs = game_state(memory);
    for(u32 i = 0; i < input->touches_count; i++) {

        os_print("pointer up index: %d\n", input->touches[i].index);

        f32 width  = os_display_width();
        f32 height = os_display_height();
        V2 pos = v2(input->touches[i].pos.x, input->touches[i].pos.y);
        pos.x /= width;
        pos.y /= height;
        pos.x -= 0.5f;
        pos.y -= 0.5f;
        pos.x *= width;
        pos.y *= -height;
        if(point_in_circle(pos, gs->button_center, gs->button_radii)) {
            gs->button_is_down[0] = false;
        } else {
            gs->joystick_is_down = false;
            gs->c_pos            = gs->s_pos;
        }
    }
}

void game_touches_move(struct Memory *memory, struct Input *input) {
    if(input->touches_count > 2) {
        return;
    }

    GameState *gs = game_state(memory);
    for(u32 i = 0; i < input->touches_count; i++) {

        //os_print("pointer move index: %d\n", input->touches[i].index);

        f32 width  = os_display_width();
        f32 height = os_display_height();
        V2 pos = v2(input->touches[i].pos.x, input->touches[i].pos.y);
        pos.x /= width;
        pos.y /= height;
        pos.x -= 0.5f;
        pos.y -= 0.5f;
        pos.x *= width;
        pos.y *= -height;
        if(point_in_circle(pos, gs->button_center, gs->button_radii)) {
            
        } else {
            gs->c_pos.x = input->touches[0].pos.x;
            gs->c_pos.y = input->touches[0].pos.y;

            f32 width  = os_display_width();
            f32 height = os_display_height();

            gs->c_pos.x /= width;
            gs->c_pos.y /= height;
            gs->c_pos.x -= 0.5f;
            gs->c_pos.y -= 0.5f;
            gs->c_pos.x *= width;
            gs->c_pos.y *= -height;
        }
    }
}
*/
