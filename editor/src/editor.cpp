struct EditorState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    
    SDL_Texture *texture;
    V2 camera;
};

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


void editor_init(EditorState *es) {

    es->camera = v2(0, 0);
    es->texture = IMG_LoadTexture(es->renderer, "../assets/rock_full.png");

}

void editor_update(EditorState *es) {
    if(key_down('d')) {
        es->camera.x += 0.1f;
    }
    if(key_down('a')) {
        es->camera.x -= 0.1f;
    }
    if(key_down('w')) {
        es->camera.y += 0.1f;
    }
    if(key_down('s')) {
        es->camera.y -= 0.1f;
    }
}

void editor_render(EditorState *es) {
    // draw a huge grid
    i32 tile_count_x = 200;
    i32 tile_count_y = 125;
    i32 htile_count_x = tile_count_x/2;
    i32 htile_count_y = tile_count_y/2;
    f32 tile_size = 0.5f;

    i32 start = -htile_count_x;
    i32 end   =  htile_count_x;
    f32 start_x = ((f32)start * tile_size) + tile_size * 0.5f;
    f32 end_x   = ((f32)end   * tile_size) + tile_size * 0.5f;
    f32 x = start_x;
    for(i32 i = start; i < end; i++) {

        f32 rel_x = x - es->camera.x;

        f32 grid_index = rel_x / ((end - start) * tile_size);        

        if(rel_x > end_x) {
            rel_x -= (end_x - start_x) * (abs((i32)grid_index) + 1);
            if(rel_x < start_x) {
                rel_x += (end_x - start_x);
            }
        }
        else if(rel_x < start_x) {
            rel_x += (end_x - start_x) * (abs((i32)grid_index) + 1);
            if(rel_x > end_x) {
                rel_x -= (end_x - start_x);
            }
        }

        rel_x = rel_x + es->camera.x;

        draw_line(es, rel_x, es->camera.y-MAP_COORDS_Y, rel_x, es->camera.y+MAP_COORDS_Y, 0xDD00AAFF);
        x += tile_size;
    }

    start = -htile_count_y;
    end   =  htile_count_y;
    f32 start_y = ((f32)start * tile_size) + tile_size * 0.5f;
    f32 end_y   = ((f32)end   * tile_size) + tile_size * 0.5f;
    f32 y = start_y;
    for(i32 i = start; i < end; i++) {

        f32 rel_y = y - es->camera.y;

        f32 grid_index = rel_y / ((end - start) * tile_size);        

        if(rel_y > end_y) {
            rel_y -= (end_y - start_y) * (abs((i32)grid_index) + 1);
            if(rel_y < start_y) {
                rel_y += (end_y - start_y);
            }
        }
        else if(rel_y < start_y) {
            rel_y += (end_y - start_y) * (abs((i32)grid_index) + 1);
            if(rel_y > end_y) {
                rel_y -= (end_y - start_y);
            }
        }

        rel_y = rel_y + es->camera.y;


        draw_line(es, es->camera.x-MAP_COORDS_X, rel_y, es->camera.x+MAP_COORDS_X, rel_y, 0xDD00AAFF);
        y += tile_size;
    }

    draw_quad(es, 0, 0, 0.5, 0.5, es->texture);
}

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

void editor_shutdown(EditorState *es) {
    SDL_DestroyTexture(es->texture);
}
