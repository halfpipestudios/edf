void state_initialize(State *state, EditorState *es,
                      void (*on_enter)(EditorState *es),
                      void (*on_exit)(EditorState *es),
                      void (*on_update)(EditorState *es),
                      void (*on_render)(EditorState *es),
                      void (*on_ui)(EditorState *es)) {
    state->es = es;
    state->on_enter = on_enter;
    state->on_exit = on_exit;
    state->on_update = state->on_update;
    state->on_render = state->on_render;
    state->on_ui = state->on_ui;
}

StateMachine state_machine_create() {
    StateMachine sm;
    memset(&sm, 0, sizeof(StateMachine));
    return sm;
}

void state_machine_push_state(StateMachine *sm, State *state) {

    assert(state->on_enter);
    assert(state->on_exit);
    assert(state->on_update);
    assert(state->on_render);
    assert(state->on_ui);

    state->on_enter(state->es);
    sm->states[sm->states_count] = state;
    sm->states_count++;
}

State *state_machine_pop_state(StateMachine *sm) {
    State *state = sm->states[sm->states_count];
    state->on_exit(state->es);
    sm->states_count--;
    return state;
}

State *state_machine_get_state(StateMachine *sm) {
    return sm->states[sm->states_count];
}

void state_machine_update(StateMachine *sm) {
    State *state = sm->states[sm->states_count];
    state->on_update(state->es);
}

void state_machine_render(StateMachine *sm) {
    State *state = sm->states[sm->states_count];
    state->on_render(state->es);
}

void state_machine_ui(StateMachine *sm) {
    State *state = sm->states[sm->states_count];
    state->on_ui(state->es);
}
