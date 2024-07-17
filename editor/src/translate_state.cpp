void translate_state_on_enter(EditorState *es) {
    printf("translate state on enter\n");
}

void translate_state_on_exit(EditorState *es) {
    printf("translate state on exit\n");
}

void translate_state_on_update(EditorState *es) {
    if(key_just_down(SDLK_ESCAPE)) {
        state_machine_pop_state(&es->sm);
    }
    if(!ImGui::IsWindowFocused() || !mouse_button_down(0) || es->mouse_wheel_down) {
        return;
    }
    Entity *entity = es->selected_entity;

    // TODO: remove this local static variable
    static V2 og_entity_pos = v2(0, 0);
    if(mouse_button_just_down(0)) {
        og_entity_pos = entity->pos;
    }

    V2 mouse_wolrd = get_mouse_world(es);
    V2 mouse_last_world = get_mouse_last_world(es);
    V2 mouse_delta  = v2_sub(mouse_wolrd, mouse_last_world);
    switch(es->ems.selected_axis) {
        case AXIS_NONE: {
            entity->pos = v2_add(entity->pos, mouse_delta);
        } break;
        case AXIS_X: {
            entity->pos.x += mouse_delta.x;
            entity->pos.y = og_entity_pos.y;
        } break;
        case AXIS_Y: {
            entity->pos.x = og_entity_pos.x;
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
