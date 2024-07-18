void translate_state_on_enter(EditorState *es) {
    printf("translate state on enter\n");
    es->selected_axis = AXIS_NONE;
}

void translate_state_on_exit(EditorState *es) {
    printf("translate state on exit\n");
}

void translate_state_on_update(EditorState *es) {
    if(key_just_down(SDLK_ESCAPE)) {
        if(es->selected_axis == AXIS_NONE) {
            state_machine_pop_state(&es->sm);
        }
        else {
            es->selected_axis = AXIS_NONE;
        }
    }

    if(key_just_down(SDLK_x)) {
        es->selected_axis = AXIS_X;
    }
    if(key_just_down(SDLK_y)) {
        es->selected_axis = AXIS_Y;
    }

    change_modify_entity_state(es);

    if(!ImGui::IsWindowFocused() || !mouse_button_down(0) || es->mouse_wheel_down) {
        return;
    }

    Entity *entity = es->selected_entity;

    V2 mouse_wolrd = get_mouse_world(es);
    V2 mouse_last_world = get_mouse_last_world(es);
    V2 mouse_delta  = v2_sub(mouse_wolrd, mouse_last_world);
    switch(es->selected_axis) {
        case AXIS_NONE: {
            entity->pos = v2_add(entity->pos, mouse_delta);
        } break;
        case AXIS_X: {
            entity->pos.x += mouse_delta.x;
        } break;
        case AXIS_Y: {
            entity->pos.y += mouse_delta.y;
        } break;
    }
}

void translate_state_on_render(EditorState *es) {

}

void translate_state_on_ui(EditorState *es) {
    // TODO: use anothe state machin for this states
    entity_modify_window(es);
    entity_property_window(es);
}
