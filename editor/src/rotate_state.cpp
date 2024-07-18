void rotate_state_on_enter(EditorState *es) {
    printf("rotate state on enter\n");
}

void rotate_state_on_exit(EditorState *es) {
    printf("rotate state on exit\n");
}

void rotate_state_on_update(EditorState *es) {
    if(key_just_down(SDLK_ESCAPE)) {
        state_machine_pop_state(&es->sm);
    }

    change_modify_entity_state(es);

    if(!ImGui::IsWindowFocused() || !mouse_button_down(0) || es->mouse_wheel_down) {
        return;
    }
    Entity *entity = es->selected_entity;

    V2 mouse_wolrd = get_mouse_world(es);
    V2 mouse_last_world = get_mouse_last_world(es);
    
    V2 curr_dir = v2_normalized(v2_sub(mouse_wolrd, entity->pos));
    V2 last_dir = v2_normalized(v2_sub(mouse_last_world, entity->pos));

    f32 curr_angle = atan2(curr_dir.y, curr_dir.x);
    f32 last_angle = atan2(last_dir.y, last_dir.x);
    f32 delta_angle = curr_angle - last_angle;
    
    entity->angle += delta_angle;
}

void rotate_state_on_render(EditorState *es) {

}

void rotate_state_on_ui(EditorState *es) {
    // TODO: use anothe state machin for this states
    entity_modify_window(es);
    entity_property_window(es);
}
