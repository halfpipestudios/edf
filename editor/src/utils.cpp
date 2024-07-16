//===========================================================================
// Utility functions
//===========================================================================
static void draw_quad(EditorState *es, f32 x, f32 y, f32 w, f32 h, SDL_Texture *texture) {

        f32 cx = es->camera.x;
        f32 cy = -es->camera.y;
        cx = (cx * METERS_TO_PIXEL); 
        cy = (cy * METERS_TO_PIXEL);

        y *= -1.0f;
        SDL_Rect dst;
        dst.w = ((w * es->zoom) * METERS_TO_PIXEL) + 1;
        dst.h = ((h * es->zoom) * METERS_TO_PIXEL) + 1;
        dst.x = ((x * es->zoom) * METERS_TO_PIXEL) + (BACK_BUFFER_WIDTH * 0.5f) - (dst.w*0.5f) - cx;
        dst.y = ((y * es->zoom) * METERS_TO_PIXEL) + (BACK_BUFFER_HEIGHT * 0.5f) - (dst.h*0.5f) - cy;
        SDL_RenderCopyEx(es->renderer, texture, 0, &dst, 0, 0, SDL_FLIP_NONE);
}

static void draw_line(EditorState *es, f32 x0, f32 y0, f32 x1, f32 y1, u32 color) {

    f32 cx = es->camera.x;
    f32 cy = -es->camera.y;
    cx = (cx * METERS_TO_PIXEL);
    cy = (cy * METERS_TO_PIXEL);

    y0 *= -1.0f;
    y1 *= -1.0f;
    x0 = (x0 * METERS_TO_PIXEL) + (BACK_BUFFER_WIDTH * 0.5f) - cx;
    y0 = (y0 * METERS_TO_PIXEL) + (BACK_BUFFER_HEIGHT * 0.5f) - cy;
    x1 = (x1 * METERS_TO_PIXEL) + (BACK_BUFFER_WIDTH * 0.5f) - cx;
    y1 = (y1 * METERS_TO_PIXEL) + (BACK_BUFFER_HEIGHT * 0.5f) - cy;
    u8 b = (u8)((color >> 24) & 0xFF); 
    u8 g = (u8)((color >> 16) & 0xFF); 
    u8 r = (u8)((color >>  8) & 0xFF); 
    u8 a = (u8)((color >>  0) & 0xFF); 
    SDL_SetRenderDrawColor(es->renderer, r, g, b, a);
    SDL_RenderDrawLine(es->renderer, x0, y0, x1, y1);
}

static f32 transform_grid_axis(f32 axis, f32 s_axis, f32 e_axis, f32 camera_axis, f32 grid_size) {
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

static void draw_grid(EditorState *es, i32 tile_count_x, i32 tile_count_y, f32 tile_size, u32 color) {
    tile_size *= es->zoom;
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

static V2 get_mouse_screen() {
    ImVec2 min = ImGui::GetWindowContentRegionMin();
    ImVec2 max = ImGui::GetWindowContentRegionMax();
    min.x += ImGui::GetWindowPos().x;
    min.y += ImGui::GetWindowPos().y;
    max.x += ImGui::GetWindowPos().x;
    max.y += ImGui::GetWindowPos().y;
    V2 result;
    result.x = get_mouse_x() - min.x;
    result.y = get_mouse_y() - min.y;
    return result;
}

static V2 get_mouse_world(EditorState *es) {
    V2 screen = get_mouse_screen();
    V2 world;
    world.x = (screen.x - (BACK_BUFFER_WIDTH*0.5f)) * PIXEL_TO_METERS;
    world.x += es->camera.x;
    world.x /= es->zoom;
    world.y = (screen.y - (BACK_BUFFER_HEIGHT*0.5f)) * PIXEL_TO_METERS;
    world.y -= es->camera.y;
    world.y /= es->zoom;
    world.y *= -1.0f;
    return world;
}

static V2 get_mouse_last_screen() {
    ImVec2 min = ImGui::GetWindowContentRegionMin();
    ImVec2 max = ImGui::GetWindowContentRegionMax();
    min.x += ImGui::GetWindowPos().x;
    min.y += ImGui::GetWindowPos().y;
    max.x += ImGui::GetWindowPos().x;
    max.y += ImGui::GetWindowPos().y;
    V2 result;
    result.x = get_mouse_last_x() - min.x;
    result.y = get_mouse_last_y() - min.y;
    return result;
}


static V2 get_mouse_last_world(EditorState *es) {
    V2 screen = get_mouse_last_screen();
    V2 world;
    world.x = (screen.x - (BACK_BUFFER_WIDTH*0.5f)) * PIXEL_TO_METERS;
    world.x += es->camera.x;
    world.x /= es->zoom;
    world.y = (screen.y - (BACK_BUFFER_HEIGHT*0.5f)) * PIXEL_TO_METERS;
    world.y -= es->camera.y;
    world.y /= es->zoom;
    world.y *= -1.0f;
    return world;
}
//===========================================================================
//===========================================================================


