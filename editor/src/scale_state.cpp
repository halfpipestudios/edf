void scale_state_on_enter(EditorState *es) {
    printf("scale state on enter\n");
}

void scale_state_on_exit(EditorState *es) {
    printf("scale state on exit\n");
}

void scale_state_on_update(EditorState *es) {
    if(key_just_down(SDLK_ESCAPE)) {
        state_machine_pop_state(&es->sm);
    }
    
    if(!ImGui::IsWindowFocused() || !mouse_button_down(0) || es->mouse_wheel_down) {
        return;
    }

    Entity *entity = es->selected_entity;

    // TODO: remove this local static variable
    static V2 og_entity_scale = v2(0, 0);
    if(mouse_button_just_down(0)) {
        og_entity_scale = entity->scale;
    }

    V2 mouse_wolrd = get_mouse_world(es);
    V2 mouse_last_world = get_mouse_last_world(es);
    V2 mouse_delta  = v2_sub(mouse_wolrd, mouse_last_world);
    switch(es->ems.selected_axis) {
        case AXIS_NONE: {
            entity->scale = v2_add(entity->scale, mouse_delta);
        } break;
        case AXIS_X: {
            entity->scale.x += mouse_delta.x;
            entity->scale.y = og_entity_scale.y;
        } break;
        case AXIS_Y: {
            entity->scale.x = og_entity_scale.x;
            entity->scale.y += mouse_delta.y;
        } break;
    }
}

void scale_state_on_render(EditorState *es) {

}

void scale_state_on_ui(EditorState *es) {
    // TODO: use anothe state machin for this states
    entity_modify_window(es);
    entity_property_window(es);
}
