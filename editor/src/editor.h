#ifndef EDITOR_H
#define EDITOR_H

struct EditorState {
    bool just_focus;

    V2 camera;
    f32 zoom;
     
    i32 mouse_wheel;
    f32 mouse_wheel_offset;
    bool mouse_wheel_down;

    EntityManager em;

    SDL_Texture *texture;

    // for internal use
    SDL_Window *window;
    SDL_Renderer *renderer;
};

#endif // EDITOR_H
