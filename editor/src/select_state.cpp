static u32 mouse_picking(EditorState *es, V2 mouse) {

    SDL_SetRenderTarget(es->renderer, es->mouse_picking_buffer);
    u32 format;
    i32 w, h;
    SDL_QueryTexture(es->mouse_picking_buffer, &format, 0, &w, &h);
    i32 bytes_per_pixel = SDL_BYTESPERPIXEL(format);

    SDL_RenderReadPixels(es->renderer, 0, format, es->mouse_picking_pixels, bytes_per_pixel * w);

    u32 color = es->mouse_picking_pixels[(i32)mouse.y * w + (i32)mouse.x] & 0xFFFFFF00;
    u32 r = (u8)((color >> 24) & 0xFF); 
    u32 g = (u8)((color >> 16) & 0xFF); 
    u32 b = (u8)((color >>  8) & 0xFF); 
    u32 a = (u8)((color >>  0) & 0xFF); 
    u32 uid = (a << 24) | (r << 16) | (g << 8) | b;
    
    SDL_SetRenderTarget(es->renderer, es->back_buffer);

    return uid;
}

static void select_entity(EditorState *es) {
    if(ImGui::IsWindowHovered()) {
        if(mouse_button_just_down(0)) {
            u32 uid = mouse_picking(es, get_mouse_screen());
            if(uid > 0) {
                // TODO: use binary search for this ...
                Entity *entity = es->em.first;
                bool founded = false;
                while(entity) {
                    if(entity->uid == uid) {
                        founded = true;
                        break;
                    }
                    entity = entity->next;
                }
                if(founded) {
                    es->selected_entity = entity;
                }
                else {
                    es->selected_entity = 0;
                }
            }
            else {
                es->selected_entity = 0;
            }
        }
    }
}

static void pop_current_state(EditorState *es) {
    State *current_state = state_machine_get_state(&es->sm);
    for(i32 i = 0; i < MODIFY_STATE_COUNT; i++) {
        if(current_state == &es->modify_states[i]) {
            state_machine_pop_state(&es->sm);
            break;
        }
    }
}

static void change_modify_entity_state(EditorState *es) {
    if(es->selected_entity) {
        if(key_just_down(SDLK_t)) {
            pop_current_state(es);
            state_machine_push_state(&es->sm, &es->modify_states[MODIFY_STATE_TRANSLATE]);
        }
        if(key_just_down(SDLK_r)) {
            pop_current_state(es);
            state_machine_push_state(&es->sm, &es->modify_states[MODIFY_STATE_ROTATE]);
        }
        if(key_just_down(SDLK_s)) {
            pop_current_state(es);
            state_machine_push_state(&es->sm, &es->modify_states[MODIFY_STATE_SCALE]);
        }
    }
}

static void entity_modify_window(EditorState *es) {
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
    ImGui::SetNextWindowClass(&window_class);
    ImGui::Begin("Entity Modify", 0,  ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    for(i32 i = 0; i < MODIFY_STATE_COUNT; i++) {
        ImVec4 tint = ImVec4(1, 1, 1, 1);
        State *current_state = state_machine_get_state(&es->sm);
        if(current_state == &es->modify_states[i]) {
            tint = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        }
        ImGui::PushID(i);
        if(ImGui::ImageButton("", es->entity_modify_buttons_textrues[i], ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint)) {
            if(es->selected_entity) {
                pop_current_state(es);
                state_machine_push_state(&es->sm, &es->modify_states[i]); 
            }
        }
        ImGui::PopID();
    }
    ImGui::End();
}

static void entity_property_window(EditorState *es) {
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
    ImGui::SetNextWindowClass(&window_class);
    ImGui::Begin("Entity", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    if(es->selected_entity) {
        Entity *entity = es->selected_entity;

        ImGui::Text("Texture:");
        ImGui::Image(entity->texture.texture, ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
        if (ImGui::BeginCombo("textures", 0, ImGuiComboFlags_NoPreview)) {
            for (i32 i = 0; i < es->texture_count; i++) {

                if(i % 2 != 0) {
                    ImGui::SameLine();
                }

                bool is_selected = entity->texture.texture == es->textures[i].texture;
                
                ImGui::PushID(i);
                if(ImGui::ImageButton(es->textures[i].texture, ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 1, ImVec4(0.0f, 0.0f, 0.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
                {
                    entity->texture = es->textures[i];
                }
                ImGui::PopID();

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Transform:");
        ImGui::InputFloat2("position", (f32 *)&entity->pos);
        ImGui::InputFloat2("scale", (f32 *)&entity->scale);
        ImGui::InputFloat("angle", &entity->angle);
    }
    else {
        ImGui::Text("there is no entity selected");
    }

    ImGui::End();
}

void select_state_on_enter(EditorState *es) {
    printf("select state on enter\n");

}

void select_state_on_exit(EditorState *es) {
    printf("select state on exit\n");
    es->selected_entity = 0;

}

void select_state_on_update(EditorState *es) {
    if(key_just_down(SDLK_ESCAPE)) {
        state_machine_pop_state(&es->sm);
    }

    select_entity(es);
    if(es->selected_entity) {
        if(key_just_down(SDLK_DELETE)) {
            entity_manager_remove_entity(&es->em, es->selected_entity);
            es->selected_entity = 0;
        }
    } 
    change_modify_entity_state(es);
}

void select_state_on_render(EditorState *es) {

}

void select_state_on_ui(EditorState *es) {
    entity_modify_window(es);
    entity_property_window(es);
}
