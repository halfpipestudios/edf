#ifndef EDITOR_H
#define EDITOR_H

enum EditorMode {
    EDITOR_MODE_SELECT_ENTITY,
    EDITOR_MODE_ADD_ENTITY,
    EDITOR_MODE_ADD_TILE,

    EDITOR_MODE_COUNT
};

enum EntityModifyMode {
    ENTITY_MODIFY_MODE_TRANSLATE,
    ENTITY_MODIFY_MODE_ROTATE,
    ENTITY_MODIFY_MODE_SCALE,

    ENTITY_MODIFY_COUNT
};


struct EditorState {
    bool just_focus;

    V2 camera;
    f32 zoom;
     
    i32 mouse_wheel;
    f32 mouse_wheel_offset;
    bool mouse_wheel_down;

    EntityManager em;
    Texture texture;

    EditorMode editor_mode;
    SDL_Texture *editor_mode_buttons_textures[EDITOR_MODE_COUNT];
    Entity *selected_entity;

    EntityModifyMode entity_modify_mode;
    SDL_Texture *entity_modify_textrues[ENTITY_MODIFY_COUNT];

    // for internal use
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *back_buffer;
    SDL_Texture *mouse_picking_buffer;
    u32 *mouse_picking_pixels;
};

#endif // EDITOR_H
