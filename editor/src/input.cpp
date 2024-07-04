struct Input {
    i32 mouse_x;
    i32 mouse_y;
    bool keys[350];
    bool mouse_buttons[3];
};

static Input g_input[2];

i32 get_mouse_x() {
    return g_input[0].mouse_x;
}

i32 get_mouse_y() {
    return g_input[0].mouse_y;
}

i32 get_mouse_last_x() {
    return g_input[1].mouse_x;
}

i32 get_mouse_last_y() {
    return g_input[1].mouse_y;
}

bool mouse_button_down(i32 button) {
    if(button >= 3) return false;
    return g_input[0].mouse_buttons[button];
}

bool mouse_button_just_down(i32 button) {
    if(button >= 3) return false;
    return g_input[0].mouse_buttons[button] && !g_input[1].mouse_buttons[button];
}

bool mouse_button_just_up(i32 button) {
    if(button >= 3) return false;
    return !g_input[0].mouse_buttons[button] && g_input[1].mouse_buttons[button];
}

bool key_down(i32 key) {
    if(key >= 350) return false;
    return g_input[0].keys[key];
}

bool key_just_down(i32 key) {
    if(key >= 350) return false;
    return g_input[0].keys[key] && !g_input[1].keys[key];
}

bool key_just_up(i32 key) {
    if(key >= 350) return false;
    return !g_input[0].keys[key] && g_input[1].keys[key];
}
