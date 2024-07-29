#ifndef EDITOR_H
#define EDITOR_H

enum EditorStates {
    EDITOR_STATE_SELECT,
    EDITOR_STATE_TILEMAP,

    EDITOR_STATE_COUNT
};

enum ModifyStates {
    MODIFY_STATE_TRANSLATE,
    MODIFY_STATE_ROTATE,
    MODIFY_STATE_SCALE,

    MODIFY_STATE_COUNT
};

#define MODIFY_SCALE_COUNT 2

enum ModifyScaleStates {
    MODIFY_SCALE_FLIP_X    = 0x01,
    MODIFY_SCALE_FLIP_Y    = 0x02
};

enum Axis {
    AXIS_X,
    AXIS_Y,
    AXIS_NONE
};

struct EditorState {
    bool just_focus;

    V2 camera;
    f32 zoom;
     
    i32 mouse_wheel;
    f32 mouse_wheel_offset;
    bool mouse_wheel_down;

    EntityManager em;
    Texture textures[128];
    char textures_names[128][MAX_PATH];
    i32 texture_count;

    Entity *selected_entity;
    Texture selected_texture;
    u8 selected_axis;
    i32 scale_state;

    // States and State Machine
    State editor_states[EDITOR_STATE_COUNT];
    State modify_states[MODIFY_STATE_COUNT];
    StateMachine sm;

    // for internal use
    SDL_Texture *editor_mode_buttons_textures[EDITOR_STATE_COUNT];
    SDL_Texture *entity_modify_buttons_textrues[MODIFY_STATE_COUNT];
    SDL_Texture *entity_modify_scale_buttons_textrues[MODIFY_SCALE_COUNT];
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *back_buffer;
    SDL_Texture *mouse_picking_buffer;
    u32 *mouse_picking_pixels;


};

#endif // EDITOR_H
