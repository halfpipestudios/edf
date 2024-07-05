//===========================================================================
// Structs
//===========================================================================
struct EditorState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool just_focus;
    
    SDL_Texture *texture;

    V2 camera;
    f32 zoom;
     
    i32 mouse_wheel;
    f32 mouse_wheel_offset;
    bool mouse_down;
};
//===========================================================================
//===========================================================================



//===========================================================================
// Utility functions
//===========================================================================

V2 screen_to_world(EditorState *es, f32 sx, f32 sy, f32 w, f32 h) {
    f32 cx = es->camera.x;
    f32 cy = -es->camera.y;
    cx = (cx * METERS_TO_PIXEL); 
    cy = (cy * METERS_TO_PIXEL);

    sx -= (w * 0.5f) - cx;
    sy -= (h * 0.5f) - cy;
    sy *= -1.0f;

    return v2((sx/es->zoom)*PIXEL_TO_METERS,
              (sy/es->zoom)*PIXEL_TO_METERS);   
}

void draw_quad(EditorState *es, f32 x, f32 y, f32 w, f32 h, SDL_Texture *texture) {

        f32 cx = es->camera.x;
        f32 cy = -es->camera.y;
        cx = (cx * METERS_TO_PIXEL); 
        cy = (cy * METERS_TO_PIXEL);

        y *= -1.0f;
        SDL_Rect dst;
        dst.w = w * METERS_TO_PIXEL;
        dst.h = h * METERS_TO_PIXEL;
        dst.x = (x * METERS_TO_PIXEL) + (WINDOW_WIDTH * 0.5f) - (dst.w*0.5f) - cx;
        dst.y = (y * METERS_TO_PIXEL) + (WINDOW_HEIGHT * 0.5f) - (dst.h*0.5f) - cy;
        SDL_RenderCopyEx(es->renderer, texture, 0, &dst, 0, 0, SDL_FLIP_NONE);
}

void draw_line(EditorState *es, f32 x0, f32 y0, f32 x1, f32 y1, u32 color) {

    f32 cx = es->camera.x;
    f32 cy = -es->camera.y;
    cx = (cx * METERS_TO_PIXEL);
    cy = (cy * METERS_TO_PIXEL);

    y0 *= -1.0f;
    y1 *= -1.0f;
    x0 = (x0 * METERS_TO_PIXEL) + (WINDOW_WIDTH * 0.5f) - cx;
    y0 = (y0 * METERS_TO_PIXEL) + (WINDOW_HEIGHT * 0.5f) - cy;
    x1 = (x1 * METERS_TO_PIXEL) + (WINDOW_WIDTH * 0.5f) - cx;
    y1 = (y1 * METERS_TO_PIXEL) + (WINDOW_HEIGHT * 0.5f) - cy;
    u8 b = (u8)((color >> 24) & 0xFF); 
    u8 g = (u8)((color >> 16) & 0xFF); 
    u8 r = (u8)((color >>  8) & 0xFF); 
    u8 a = (u8)((color >>  0) & 0xFF); 
    SDL_SetRenderDrawColor(es->renderer, r, g, b, a);
    SDL_RenderDrawLine(es->renderer, x0, y0, x1, y1);
}

f32 transform_grid_axis(f32 axis, f32 s_axis, f32 e_axis, f32 camera_axis, f32 grid_size) {
    f32 r_axis = axis - camera_axis;
    f32 grid_index = r_axis / grid_size;        
    if(r_axis > e_axis) {
        r_axis -= (e_axis - s_axis) * (abs((i32)grid_index) + 1);
        if(r_axis < s_axis) {
            r_axis += (e_axis - s_axis);
        }
    }
    else if(r_axis < s_axis) {
        r_axis += (e_axis - s_axis) * (abs((i32)grid_index) + 1);
        if(r_axis > e_axis) {
            r_axis -= (e_axis - s_axis);
        }
    }
    return r_axis + camera_axis;
}

void draw_gird(EditorState *es, i32 tile_count_x, i32 tile_count_y, f32 tile_size, u32 color) {
    i32 htile_count_x = tile_count_x/2;
    i32 htile_count_y = tile_count_y/2;
    // draw vertical lines
    i32 start = -htile_count_x;
    i32 end   =  htile_count_x;
    f32 grid_size = ((end - start) * tile_size);
    f32 start_x = ((f32)start * tile_size) + tile_size * 0.5f;
    f32 end_x   = ((f32)end   * tile_size) + tile_size * 0.5f;
    f32 x = start_x;
    for(i32 i = start; i < end; i++) {
        f32 rel_x = transform_grid_axis(x, start_x, end_x, es->camera.x, grid_size);
        draw_line(es, rel_x, es->camera.y-(MAP_COORDS_Y*0.5f), rel_x, es->camera.y+(MAP_COORDS_Y*0.5f), color);
        x += tile_size;
    }
    // draw horizontal lines
    start = -htile_count_y;
    end   =  htile_count_y;
    grid_size = ((end - start) * tile_size);
    f32 start_y = ((f32)start * tile_size) + tile_size * 0.5f;
    f32 end_y   = ((f32)end   * tile_size) + tile_size * 0.5f;
    f32 y = start_y;
    for(i32 i = start; i < end; i++) {
        f32 rel_y = transform_grid_axis(y, start_y, end_y, es->camera.y, grid_size);
        draw_line(es, es->camera.x-(MAP_COORDS_X*0.5f), rel_y, es->camera.x+(MAP_COORDS_X*0.5f), rel_y, color);
        y += tile_size;
    }
}

//===========================================================================
//===========================================================================


//===========================================================================
// Entry point of the level editor
//===========================================================================
void editor_init(EditorState *es) {
    es->just_focus = false;
    es->camera = v2(0, 0);
    es->mouse_wheel = 0;
    es->mouse_wheel_offset = 1.0f;
    es->zoom = es->mouse_wheel_offset * es->mouse_wheel_offset;
    es->mouse_down = false;

    es->texture = IMG_LoadTexture(es->renderer, "../assets/rock_full.png");
}

void editor_shutdown(EditorState *es) {
    SDL_DestroyTexture(es->texture);
}
//===========================================================================
//===========================================================================



//===========================================================================
// Level View (SDL and ImGui)
//===========================================================================
void editor_update(EditorState *es) {
    if(ImGui::IsWindowFocused()) {
        if(mouse_button_down(0)) {
            f32 x_delta = (f32)get_mouse_delta_x();
            f32 y_delta = (f32)get_mouse_delta_y();
            es->camera.x -= x_delta*PIXEL_TO_METERS;
            es->camera.y += y_delta*PIXEL_TO_METERS;
        }
    }


    ImVec2 min = ImGui::GetWindowContentRegionMin();
    ImVec2 max = ImGui::GetWindowContentRegionMax();
    min.x += ImGui::GetWindowPos().x;
    min.y += ImGui::GetWindowPos().y;
    max.x += ImGui::GetWindowPos().x;
    max.y += ImGui::GetWindowPos().y;
    f32 rel_x = get_mouse_x() - min.x;
    f32 rel_y = get_mouse_y() - min.y;


    // handle zoom
    if(ImGui::IsWindowHovered()) {
        V2 mouse_pos_pre_zoom = screen_to_world(es, rel_x, rel_y, WINDOW_WIDTH, WINDOW_HEIGHT);
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
        V2 mouse_pos_post_zoom = screen_to_world(es, rel_x, rel_y, WINDOW_WIDTH, WINDOW_HEIGHT);
        es->camera.x += (mouse_pos_pre_zoom.x - mouse_pos_post_zoom.x)*es->zoom;
        es->camera.y += (mouse_pos_pre_zoom.y - mouse_pos_post_zoom.y)*es->zoom;
    }
}

void editor_render(EditorState *es) {

    f32 tile_size = 1.0f * es->zoom;

    f32 y = 0;
    for(i32 j = -5; j < 5; j++) {
        f32 x = 0;
        for(i32 i = -5; i < 5; i++) {
            draw_quad(es, x, y, tile_size, tile_size, es->texture);
            x += tile_size;
        }
        y += tile_size;
    }

    if(es->mouse_wheel_offset > 0.23f) {
        draw_gird(es, 200, 125, tile_size, 0xDD00AAFF);
    }
    // draw the x and y axis for reference of the origin (0, 0)
    draw_line(es, es->camera.x-(MAP_COORDS_X*0.5f), 0, es->camera.x+(MAP_COORDS_X*0.5f), 0, 0x0000FFFF);
    draw_line(es, 0, es->camera.y-(MAP_COORDS_Y*0.5f), 0, es->camera.y+(MAP_COORDS_Y*0.5f), 0x00FF00FF);
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
    // hello world window
    {
        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
        if(ImGui::Button("Button 1")) {
            printf("Hola juani\n");    
        }
        ImGui::End();
    }
}
//===========================================================================
//===========================================================================

