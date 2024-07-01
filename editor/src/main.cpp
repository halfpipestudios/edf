#include <stdio.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <edf_common.h>

struct Input {
    i32 mouse_x;
    i32 mouse_y;
    bool keys[350];
    bool mouse_buttons[3];
};

static SDL_Window   *g_window;
static SDL_Renderer *g_renderer;
static bool g_running;
static Input g_input[2];

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

i32 get_mouse_x() {
    return g_input[0].mouse_x;
}

i32 get_mouse_y() {
    return g_input[0].mouse_y;
}

i32 get_mouse_last_x() {
    return g_input[1].mouse_x;
}

i32 get_mouse_last_y() {
    return g_input[1].mouse_y;
}

bool mouse_button_down(i32 button) {
    if(button >= 3) return false;
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse) return false;
    return g_input[0].mouse_buttons[button];
}

bool mouse_button_just_down(i32 button) {
    if(button >= 3) return false;
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse) return false;
    return g_input[0].mouse_buttons[button] && !g_input[1].mouse_buttons[button];
}

bool mouse_button_just_up(i32 button) {
    if(button >= 3) return false;
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse) return false;
    return !g_input[0].mouse_buttons[button] && g_input[1].mouse_buttons[button];
}

bool key_down(i32 key) {
    if(key >= 350) return false;
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureKeyboard) return false;
    return g_input[0].keys[key];
}

bool key_just_down(i32 key) {
    if(key >= 350) return false;
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureKeyboard) return false;
    return g_input[0].keys[key] && !g_input[1].keys[key];
}

bool key_just_up(i32 key) {
    if(key >= 350) return false;
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureKeyboard) return false;
    return !g_input[0].keys[key] && g_input[1].keys[key];
}

static i32 sdl_mouse_event_to_index(SDL_MouseButtonEvent event) {
    i32 index = -1;
    switch(event.button) {
        case SDL_BUTTON_LEFT: index = 0; break; 
        case SDL_BUTTON_MIDDLE: index = 1; break; 
        case SDL_BUTTON_RIGHT: index = 2; break; 
        default: index = 0;
    }
    return index;
}

i32 main(void) {
    SDL_Init(SDL_INIT_EVERYTHING);

    g_window = SDL_CreateWindow("EDF Editor",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                WINDOW_WIDTH, WINDOW_HEIGHT, 
                                SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN|SDL_WINDOW_MAXIMIZED);
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

    SDL_Texture *back_buffer = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                                 WINDOW_WIDTH, WINDOW_HEIGHT);

    SDL_Texture *rocks_full = IMG_LoadTexture(g_renderer, "../assets/rock_full.png");
    SDL_Texture *rocks_corner = IMG_LoadTexture(g_renderer, "../assets/rocks_corner.png");
    SDL_Texture *rocks_flat = IMG_LoadTexture(g_renderer, "../assets/rocks_flat.png");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    
    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();
    ImGui_ImplSDL2_InitForSDLRenderer(g_window, g_renderer);
    ImGui_ImplSDLRenderer2_Init(g_renderer);
    
    f32 col1[3] = { 0, 0, 0 };

    g_running = true;
    while(g_running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);            
            if(event.type == SDL_QUIT) {
                g_running = false;
            }
            if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.sym < 350) {
                    g_input[0].keys[event.key.keysym.sym] = true;
                }
            }
            if(event.type == SDL_KEYUP) {
                if(event.key.keysym.sym < 350) {
                    g_input[0].keys[event.key.keysym.sym] = false;
                }
            }
            if(event.type == SDL_MOUSEBUTTONDOWN) {
                g_input[0].mouse_buttons[sdl_mouse_event_to_index(event.button)] = true;
            }
            if(event.type == SDL_MOUSEBUTTONUP) {
                g_input[0].mouse_buttons[sdl_mouse_event_to_index(event.button)] = false;
            }
        }
        SDL_GetMouseState(&g_input->mouse_x, &g_input->mouse_y);



        if(mouse_button_just_down(0)) {
            printf("mouse just down\n");
        }


        SDL_SetRenderTarget(g_renderer, back_buffer);
        SDL_SetRenderDrawColor(g_renderer, 180, 200, 180, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(g_renderer);
        SDL_Rect dst;
        dst.w = 100;
        dst.h = 100;
        for(i32 y = 0; y < WINDOW_HEIGHT/100; y++) {
            for(i32 x = 0; x < WINDOW_WIDTH/100; x++) {
                dst.x = x * 102;
                dst.y = y * 102;
                SDL_RenderCopyEx(g_renderer, rocks_full, 0, &dst, 0, 0, SDL_FLIP_NONE);
            }
        }

        SDL_SetRenderTarget(g_renderer, 0);
        SDL_SetRenderDrawColor(g_renderer, col1[0]*255, col1[1]*255, col1[2]*255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(g_renderer);

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Start ImGui Docking
        ImGuiWindowFlags windowFlags = {};
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        static bool condition = true;
        ImGui::Begin("Dockspace demo", &condition, windowFlags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        ImGui::DockSpace(ImGui::GetID("Dockspace"));

        // draw the back buffer
        ImGui::Begin("Game Viewport");
        ImVec4 border_col = ImGui::GetStyleColorVec4(ImGuiCol_Border);
        ImGui::Image(back_buffer, ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), border_col);
        ImGui::End();

        // TODO: do imgui demo
        static bool show_demo = true; 
        ImGui::ShowDemoWindow(&show_demo);

       // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::ColorEdit3("background color", col1);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }


        ImGui::End();
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        SDL_RenderSetScale(g_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), g_renderer);

        SDL_RenderPresent(g_renderer); 

        // swap the input pointer
        g_input[1] = g_input[0];

    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return(0);
};
