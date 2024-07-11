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
        //SDL_SetTextureColorMod(entity->texture, r, g, b);
        draw_quad(es, 
                  entity->pos.x, entity->pos.y,
                  entity->scale.x, entity->scale.y,
                  entity->texture);
        entity = entity->next;
    }
}

static void draw_grid_x_y(EditorState *es) {
    if(es->mouse_wheel_offset > 0.25f) {
        draw_grid(es, 400, 225, 0.5f, 0xDD00AAFF);
    }
    // draw the x and y axis for reference of the origin (0, 0)
    draw_line(es, es->camera.x-(MAP_COORDS_X*0.5f), 0, es->camera.x+(MAP_COORDS_X*0.5f), 0, 0x0000FFFF);
    draw_line(es, 0, es->camera.y-(MAP_COORDS_Y*0.5f), 0, es->camera.y+(MAP_COORDS_Y*0.5f), 0x00FF00FF);
}

struct Texture {
    i32 w, h;
    u32 format;
    SDL_Texture *texture;
    SDL_Texture *mask;
};


Texture load_texture_and_mask(EditorState *es, const char *path) {
    Texture texture;
    // load the texture
    SDL_Surface *surface = IMG_Load(path);
    texture.texture = SDL_CreateTextureFromSurface(es->renderer, surface);
    
    SDL_LockSurface(surface);

    i32 bytes_per_pixel = surface->format->BytesPerPixel;
    u32 a_mask = surface->format->Amask;
    i32 width = surface->w;
    i32 height = surface->h;
    u32 *pixel = (u32 *)surface->pixels;
    for(i32 y = 0; y < height; y++) {
        for(i32 x = 0; x < width; x++) {
            u32 color = pixel[y * width + x];
            pixel[y * width + x] = ~a_mask | (color & a_mask);
        }
    }

    SDL_UnlockSurface(surface);

    texture.mask = SDL_CreateTextureFromSurface(es->renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
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


    Texture test = load_texture_and_mask(es, "../assets/rocks_corner.png");

    es->texture = test.texture;


}

void editor_shutdown(EditorState *es) {
    SDL_DestroyTexture(es->texture);
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
    // add entitites
    if(ImGui::IsWindowHovered()) {
        static u32 uid = 0x0000000;
        if(mouse_button_just_down(0)) {
            Entity *entity = entity_manager_add_entity(&es->em);
            V2 mouse_w = get_mouse_world(es);
            entity->pos.x = roundf(mouse_w.x / 0.5f) * 0.5f;
            entity->pos.y = roundf(mouse_w.y / 0.5f) * 0.5f;
            entity->scale = v2(0.5f, 0.5f);
            entity->texture = es->texture;
            entity->uid = uid;
            uid++;
        }
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
void editor_ui(EditorState *es) {
    // demo window
    {
        static bool show_demo = true; 
        ImGui::ShowDemoWindow(&show_demo);
    }
}
//===========================================================================
//===========================================================================

