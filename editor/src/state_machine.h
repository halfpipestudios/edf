#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

struct EditorState;

struct State {
    EditorState *es;
    void (*on_enter)(EditorState *es);
    void (*on_exit)(EditorState *es);
    void (*on_update)(EditorState *es);
    void (*on_render)(EditorState *es);
    void (*on_ui)(EditorState *es);
};

struct StateMachine {
    State *states[100];
    i32 states_count;
};

#endif // STATE_MACHINE_H
