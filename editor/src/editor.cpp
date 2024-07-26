static void draw_all_entities(EditorState *es) {
    Entity *entity = es->em.first;
    while(entity) {
        u8 r = (entity->uid >> 16) & 0xFF;
        u8 g = (entity->uid >>  8) & 0xFF;
        u8 b = (entity->uid >>  0) & 0xFF;
        
        draw_quad(es, 
                  entity->pos.x, entity->pos.y,
                  entity->scale.x, entity->scale.y,
                  entity->angle,
                  entity->texture.texture);

        entity = entity->next;
    }
}

static void draw_all_entities_collision(EditorState *es) {

    Entity *entity = es->em.first;
    while(entity) {
        if(entity->components & ENTITY_COLLISION_COMPONENT) {
            V2 offset = entity->collision.offset;
            if(entity->collision.type == COLLISION_TYPE_CIRLCE) {
                Circle *circle = &entity->collision.circle;
                draw_circle_world(es, entity->pos, circle->r, offset, entity->angle);
            }
            else if(entity->collision.type == COLLISION_TYPE_AABB) {
                AABB *aabb = &entity->collision.aabb;
                draw_aabb_world(es, entity->pos, offset, aabb->min, aabb->max, entity->angle);
            }
            else if(entity->collision.type == COLLISION_TYPE_OBB) {
                OBB *obb = &entity->collision.obb;
                draw_obb_world(es, entity->pos, offset, obb->he, entity->angle + obb->r);
            }
        }
        entity = entity->next;
    }

}

static void draw_selected_entity_gizmo(EditorState *es) {
    // reference for entity orientation
    if(es->selected_entity) {
        Entity *entity = es->selected_entity;
        V2 o = entity->pos;
        V2 x = v2(cosf(entity->angle), sinf(entity->angle));
        V2 y = v2(-x.y, x.x);
        x = v2_scale(x, 1.0f/es->zoom);
        y = v2_scale(y, 1.0f/es->zoom);
        V2 a = v2_add(o, x);
        V2 b = v2_add(o, y);
        draw_line_world(es, o.x, o.y, a.x, a.y, 0x0000FFFF);
        draw_line_world(es, o.x, o.y, b.x, b.y, 0x00FF00FF);
    }
}

static void draw_grid_x_y(EditorState *es) {
    if(es->mouse_wheel_offset > 0.25f) {
        draw_grid(es, 400, 225, 0.5f, 0x413620FF);
    }
    // draw the x and y axis for reference of the origin (0, 0)
    draw_line(es, es->camera.x-(MAP_COORDS_X*0.5f), 0, es->camera.x+(MAP_COORDS_X*0.5f), 0, 0x0000FFFF);
    draw_line(es, 0, es->camera.y-(MAP_COORDS_Y*0.5f), 0, es->camera.y+(MAP_COORDS_Y*0.5f), 0x00FF00FF);
}

static Texture load_texture_and_mask(EditorState *es, const char *path) {
    Texture texture;
    // load the texture
    SDL_Surface *surface = IMG_Load(path);
    texture.texture = SDL_CreateTextureFromSurface(es->renderer, surface);
    
    // create the mask for the mouse picking
    SDL_LockSurface(surface);
    i32 bytes_per_pixel = surface->format->BytesPerPixel;
    u32 a_mask = surface->format->Amask;
    i32 width = surface->w;
    i32 height = surface->h;
    u32 *pixel = (u32 *)surface->pixels;
    for(i32 y = 0; y < height; y++) {
        for(i32 x = 0; x < width; x++) {
            u32 color = pixel[y * width + x];
            if((color & a_mask) > 0) {
                pixel[y * width + x] = 0xFFFFFFFF;
            }
            else {
                pixel[y * width + x] = 0x00000000;
            }
        }
    }
    SDL_UnlockSurface(surface);
    texture.mask = SDL_CreateTextureFromSurface(es->renderer, surface);
    SDL_FreeSurface(surface);
    
    return texture;
}

static void process_panning(EditorState *es) {
    if(ImGui::IsWindowHovered()) {
        if(mouse_button_just_down(1)) { 
            es->mouse_wheel_down = true;
        }
    }
    if(mouse_button_just_up(1)) {
        es->mouse_wheel_down = false;    
    }
    if(es->mouse_wheel_down) {
        f32 x_delta = (f32)get_mouse_delta_x();
        f32 y_delta = (f32)get_mouse_delta_y();
        es->camera.x -= x_delta*PIXEL_TO_METERS;
        es->camera.y += y_delta*PIXEL_TO_METERS;
    }
}

static void process_zoom(EditorState *es) {
    if(ImGui::IsWindowHovered()) {
        V2 mouse_pos_pre_zoom = get_mouse_world(es);
        if(es->mouse_wheel != 0)
        {
            if(es->mouse_wheel > 0)
            {
                es->mouse_wheel_offset *= 1.1f;
            }
            else
            {
                es->mouse_wheel_offset *= 0.9f;
            }
            es->mouse_wheel_offset = clamp(es->mouse_wheel_offset, 0.15f, 8.0f);
            es->zoom = es->mouse_wheel_offset * es->mouse_wheel_offset;
        } 
        V2 mouse_pos_post_zoom = get_mouse_world(es);
        es->camera.x += (mouse_pos_pre_zoom.x - mouse_pos_post_zoom.x)*es->zoom;
        es->camera.y += (mouse_pos_pre_zoom.y - mouse_pos_post_zoom.y)*es->zoom;
    }
}

//===========================================================================
// Entry point of the level editor
//===========================================================================
void editor_init(EditorState *es) {
    es->just_focus = false;
    
    es->mouse_wheel = 0;
    es->mouse_wheel_offset = 1.0f;
    es->mouse_wheel_down = false;

    es->camera = v2(0, 0);
    es->zoom = es->mouse_wheel_offset * es->mouse_wheel_offset;

    es->em = entity_manager_create(1000);

    es->editor_mode_buttons_textures[0] = IMG_LoadTexture(es->renderer, "../assets/select_button.png");
    es->editor_mode_buttons_textures[1] = IMG_LoadTexture(es->renderer, "../assets/entity_button.png");
    es->selected_entity = 0;

    es->entity_modify_buttons_textrues[0] = IMG_LoadTexture(es->renderer, "../assets/translate_button.png");
    es->entity_modify_buttons_textrues[1] = IMG_LoadTexture(es->renderer, "../assets/rotation_button.png");
    es->entity_modify_buttons_textrues[2] = IMG_LoadTexture(es->renderer, "../assets/scale_button.png");
    es->selected_axis = AXIS_NONE;


    // Load all the exture in the game asset folder
    // Get All file in a directory test
    namespace fs = std::filesystem;
    // path to the directory
    std::string path = "../../game/assets";
    // looping until all the items in the dir are exhausted
    es->texture_count = 0;
    for(const auto& entry : fs::directory_iterator(path)) {
        // Converting the path to const char * in the
        // subsequent lines
        fs::path outfilename = entry.path();
        std::string outfilename_str = outfilename.string();
        if (outfilename_str.find(".png") != std::string::npos) {
            const char *path = outfilename_str.c_str();
            es->textures[es->texture_count] = load_texture_and_mask(es, path);
            es->textures[es->texture_count].index = es->texture_count;

            const char *name = path + (outfilename_str.find('\\') + 1); 
            memset(es->textures_names[es->texture_count], 0, MAX_PATH);
            memcpy(es->textures_names[es->texture_count], name, strlen(name));
            es->texture_count++;

        }
    }
    es->selected_texture = es->textures[0];

    state_initialize(&es->editor_states[EDITOR_STATE_SELECT], es, 
                          select_state_on_enter,
                          select_state_on_exit,
                          select_state_on_update,
                          select_state_on_render,
                          select_state_on_ui);

    state_initialize(&es->editor_states[EDITOR_STATE_TILEMAP], es, 
                          tilemap_state_on_enter,
                          tilemap_state_on_exit,
                          tilemap_state_on_update,
                          tilemap_state_on_render,
                          tilemap_state_on_ui);

    state_initialize(&es->modify_states[MODIFY_STATE_TRANSLATE], es, 
                          translate_state_on_enter,
                          translate_state_on_exit,
                          translate_state_on_update,
                          translate_state_on_render,
                          translate_state_on_ui);

    state_initialize(&es->modify_states[MODIFY_STATE_ROTATE], es,
                          rotate_state_on_enter,
                          rotate_state_on_exit,
                          rotate_state_on_update,
                          rotate_state_on_render,
                          rotate_state_on_ui);

    state_initialize(&es->modify_states[MODIFY_STATE_SCALE], es, 
                          scale_state_on_enter,
                          scale_state_on_exit,
                          scale_state_on_update,
                          scale_state_on_render,
                          scale_state_on_ui);

    es->sm = state_machine_create();

}

void editor_shutdown(EditorState *es) {
    SDL_DestroyTexture(es->editor_mode_buttons_textures[0]);
    SDL_DestroyTexture(es->editor_mode_buttons_textures[1]);

    SDL_DestroyTexture(es->entity_modify_buttons_textrues[0]);
    SDL_DestroyTexture(es->entity_modify_buttons_textrues[1]);
    SDL_DestroyTexture(es->entity_modify_buttons_textrues[2]);

    for(i32 i = 0; i < es->texture_count; i++) {
        SDL_DestroyTexture(es->textures[i].texture);
        SDL_DestroyTexture(es->textures[i].mask);
    }
    entity_manager_destroy(&es->em);
}
//===========================================================================
//===========================================================================


//===========================================================================
// Level View (SDL and ImGui)
//===========================================================================
void editor_update(EditorState *es) {
    process_panning(es);
    state_machine_update(&es->sm);
    process_zoom(es);

    // TODO: make a state and ui for this 
    if(key_just_down(SDLK_p)) {
        entity_manager_serialize(&es->em, "../levels/test.level", es);
    }

}

void editor_render(EditorState *es) {
    draw_all_entities(es);
    state_machine_render(&es->sm);
    draw_grid_x_y(es);
    draw_all_entities_collision(es);
    draw_selected_entity_gizmo(es);

}
//===========================================================================
//===========================================================================


//==================================================================================
// ImGui UI (ImGui only Do Not use SDL here, only pass SDL_Texture to ImGui::Image)
//==================================================================================
static void editor_mode_window(EditorState *es) {
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
    ImGui::SetNextWindowClass(&window_class);
    ImGui::Begin("Editor Mode Selector", 0,  ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    for(i32 i = 0; i < EDITOR_STATE_COUNT; i++) {
        ImVec4 tint = ImVec4(1, 1, 1, 1);
        State *current_state = state_machine_get_state(&es->sm);
        if(current_state == &es->editor_states[i]) {
            tint = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        }
        ImGui::PushID(i);
        if(ImGui::ImageButton("", es->editor_mode_buttons_textures[i], ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint)) {
            state_machine_clear(&es->sm);
            state_machine_push_state(&es->sm, &es->editor_states[i]);
        }
        ImGui::PopID();
        ImGui::SameLine();
    }
    ImGui::End();
}

void editor_ui(EditorState *es) {
    editor_mode_window(es);
    state_machine_ui(&es->sm);
}
//===========================================================================
//===========================================================================

