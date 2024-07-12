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

static void draw_all_entities(EditorState *es) {
    Entity *entity = es->em.first;
    while(entity) {
        u8 r = (entity->uid >> 16) & 0xFF;
        u8 g = (entity->uid >>  8) & 0xFF;
        u8 b = (entity->uid >>  0) & 0xFF;
        if(entity == es->selected_entity) {
            SDL_SetTextureColorMod(entity->texture.texture, 0, 255, 0);
        }
        else {
            SDL_SetTextureColorMod(entity->texture.texture, 255, 255, 255);
        }
        draw_quad(es, 
                  entity->pos.x, entity->pos.y,
                  entity->scale.x, entity->scale.y,
                  entity->texture.texture);
        entity = entity->next;
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

static void add_tile(EditorState *es) {
    if(ImGui::IsWindowHovered()) {
        static u32 uid = 1;
        if(mouse_button_just_down(0)) {
            Entity *entity = entity_manager_add_entity(&es->em);
            V2 mouse_w = get_mouse_world(es);
            entity->pos.x = roundf(mouse_w.x / 0.5f) * 0.5f;
            entity->pos.y = roundf(mouse_w.y / 0.5f) * 0.5f;
            entity->scale = v2(0.5f, 0.5f);
            entity->texture = es->texture;
            assert((uid > 0) && (uid < 0xFF000000));
            entity->uid = uid;
            uid++;
        }

    }
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



static void modify_entity(EditorState *es, Entity *entity) {
    if(key_just_down(127)) {
        entity_manager_remove_entity(&es->em, entity);
        es->selected_entity = 0;
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

    es->texture = load_texture_and_mask(es, "../assets/rocks_corner.png");

    es->editor_mode_buttons_textures[0] = IMG_LoadTexture(es->renderer, "../assets/select_button.png");
    es->editor_mode_buttons_textures[1] = IMG_LoadTexture(es->renderer, "../assets/entity_button.png");
    es->editor_mode_buttons_textures[2] = IMG_LoadTexture(es->renderer, "../assets/tile_button.png");
    es->editor_mode = EDITOR_MODE_ADD_TILE;
    es->selected_entity = 0;

    es->entity_modify_textrues[0] = IMG_LoadTexture(es->renderer, "../assets/translate_button.png");
    es->entity_modify_textrues[1] = IMG_LoadTexture(es->renderer, "../assets/rotation_button.png");
    es->entity_modify_textrues[2] = IMG_LoadTexture(es->renderer, "../assets/scale_button.png");
    es->entity_modify_mode = ENTITY_MODIFY_MODE_TRANSLATE;
}

void editor_shutdown(EditorState *es) {
    SDL_DestroyTexture(es->editor_mode_buttons_textures[0]);
    SDL_DestroyTexture(es->editor_mode_buttons_textures[1]);
    SDL_DestroyTexture(es->editor_mode_buttons_textures[2]);
    SDL_DestroyTexture(es->texture.texture);
    SDL_DestroyTexture(es->texture.mask);
    entity_manager_destroy(&es->em);
}
//===========================================================================
//===========================================================================


//===========================================================================
// Level View (SDL and ImGui)
//===========================================================================
void editor_update(EditorState *es) {
    process_panning(es);
    process_zoom(es);
        
    switch(es->editor_mode) {
        case EDITOR_MODE_SELECT_ENTITY: {
            select_entity(es);
            if(es->selected_entity) {
                modify_entity(es, es->selected_entity);
            }
        } break;
        case EDITOR_MODE_ADD_ENTITY: {
            // TODO: ...
        } break;
        case EDITOR_MODE_ADD_TILE: {
            add_tile(es);
        } break;
        default: {
            assert(!"editor mode not handle!");
        } break;
    }
}

void editor_render(EditorState *es) {
    draw_all_entities(es);
    draw_grid_x_y(es);
}
//===========================================================================
//===========================================================================


//===========================================================================
// ImGui UI (ImGui only Do Not use SDL here, only pass SDL_Texture to ImGui::Image)
//===========================================================================

void editor_mode_window(EditorState *es) {
    ImGui::Begin("Editor Mode Selector", 0,  ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    for(i32 i = 0; i < EDITOR_MODE_COUNT; i++) {
        ImVec4 tint = ImVec4(1, 1, 1, 1);
        if(es->editor_mode == i) {
            tint = ImVec4(0.5f, 0.5f, 0.5f, 1);
        }
        ImGui::PushID(i);
        if(ImGui::ImageButton("", es->editor_mode_buttons_textures[i], ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint)) {
            if(es->editor_mode != i) {
                es->editor_mode = (EditorMode)i;
            }
            
        }
        ImGui::PopID();
        ImGui::SameLine();
    }
    ImGui::End();
}

void entity_modify_window(EditorState *es) {
    ImGui::Begin("Entity Modify", 0,  ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    for(i32 i = 0; i < ENTITY_MODIFY_COUNT; i++) {
        ImVec4 tint = ImVec4(1, 1, 1, 1);
        if(es->entity_modify_mode == i) {
            tint = ImVec4(0.5f, 0.5f, 0.5f, 1);
        }
        ImGui::PushID(i);
        if(ImGui::ImageButton("", es->entity_modify_textrues[i], ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint)) {
            if(es->entity_modify_mode != i) {
                es->entity_modify_mode = (EntityModifyMode)i;
            }
            
        }
        ImGui::PopID();
    }
    ImGui::End();
}

void editor_ui(EditorState *es) {
    editor_mode_window(es);
    entity_modify_window(es);

#if 0
    // demo window
    {
        static bool show_demo = true; 
        ImGui::ShowDemoWindow(&show_demo);
    }
#endif
}
//===========================================================================
//===========================================================================

