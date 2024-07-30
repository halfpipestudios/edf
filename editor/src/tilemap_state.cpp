static void set_entity_scale_base_on_editor_scale_state(EditorState *es, Entity *entity) {
    f32 t = (f32)((es->scale_state & MODIFY_SCALE_FLIP_X) != 0);
    entity->scale.x = fabsf(entity->scale.x) * lerp(1.0f, -1.0f, t);
    t = (f32)((es->scale_state & MODIFY_SCALE_FLIP_Y) != 0);
    entity->scale.y = fabsf(entity->scale.y) * lerp(1.0f, -1.0f, t);
}

static void entity_modify_scale_window(EditorState *es) {
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
    ImGui::SetNextWindowClass(&window_class);
    ImGui::Begin("Entity Modify Scale", 0,  ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    for(i32 i = 0; i < MODIFY_SCALE_COUNT; i++) {
        ImVec4 tint = ImVec4(1, 1, 1, 1);
        if(es->scale_state & bit(i)) {
            tint = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        }
        ImGui::PushID(i);
        if(ImGui::ImageButton("", es->entity_modify_scale_buttons_textrues[i], ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint)) {
            es->scale_state ^= bit(i);
        }
        ImGui::PopID();
    }
    ImGui::End();
}

static void texture_selector_window(EditorState *es) {
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
    ImGui::SetNextWindowClass(&window_class);
    ImGui::Begin("Textures", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    ImGuiStyle& style = ImGui::GetStyle();
    i32 buttons_count = es->texture_count;
    f32 window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    for (i32 i = 0; i < buttons_count; i++)
    {
        ImTextureID textureId = es->textures[i].texture;
        f32 uMin = 0;
        f32 vMin = 0;

        f32 uMax = 1;
        f32 vMax = 1;

        ImVec2 uvMin = ImVec2(uMin, vMin);
        ImVec2 uvMax = ImVec2(uMax, vMax);

        ImVec4 tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 backCol = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        
        ImGui::PushID(i);
        if(ImGui::ImageButton(textureId, ImVec2(64, 64), uvMin, uvMax, 1, backCol, tintCol))
        {
            es->selected_texture = es->textures[i];
        }

        f32 max = ImGui::GetItemRectMax().x;
        f32 min = ImGui::GetItemRectMin().x;

        f32 last_button_x2 = ImGui::GetItemRectMax().x;
        f32 next_button_x2 = last_button_x2 + style.ItemSpacing.x + (max - min);
        
        if (i + 1 < buttons_count && next_button_x2 < window_visible_x2)
            ImGui::SameLine();


        ImGui::PopID();
    }

    ImGui::End();
}


void tilemap_state_on_enter(EditorState *es) {
    printf("tilemap state on enter\n");

}

void tilemap_state_on_exit(EditorState *es) {
    printf("tilemap state on exit\n");
}

void tilemap_state_on_update(EditorState *es) {
    if(key_just_down(SDLK_ESCAPE)) {
        state_machine_pop_state(&es->sm);
    }

    if(ImGui::IsWindowHovered()) {
        // TODO: remove this static from here becouse this is not
        // the only place we are going to add entitites
        static u32 uid = 1;
        if(mouse_button_just_down(0)) {
            Entity *entity = entity_manager_add_entity(&es->em);
            V2 mouse_w = get_mouse_world(es);
            entity->pos.x = roundf(mouse_w.x / 0.5f) * 0.5f;
            entity->pos.y = roundf(mouse_w.y / 0.5f) * 0.5f;
            entity->scale = v2(0.5f, 0.5f);
            set_entity_scale_base_on_editor_scale_state(es, entity);
            entity->texture = es->selected_texture;
            assert((uid > 0) && (uid < 0xFF000000));
            entity->uid = uid;
            uid++;

            entity_add_components(entity, ENTITY_RENDER_COMPONENT);
        }
    }
}

void tilemap_state_on_render(EditorState *es) {

}

void tilemap_state_on_ui(EditorState *es) {
    texture_selector_window(es);
    entity_modify_scale_window(es);
}