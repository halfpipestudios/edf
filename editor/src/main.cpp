#include <stdio.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Headers
#include <edf_common.h>
#include <edf_math.h>
#include "common.h"
// Srouces
#include "editor.cpp"

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
    EditorState *es = (EditorState *)malloc(sizeof(EditorState));

    SDL_Init(SDL_INIT_EVERYTHING);
    es->window = SDL_CreateWindow("EDF Editor",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                WINDOW_WIDTH, WINDOW_HEIGHT, 
                                SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN|SDL_WINDOW_MAXIMIZED);
    es->renderer = SDL_CreateRenderer(es->window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture *back_buffer = SDL_CreateTexture(es->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                                 WINDOW_WIDTH, WINDOW_HEIGHT);


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    
    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();
    ImGui_ImplSDL2_InitForSDLRenderer(es->window, es->renderer);
    ImGui_ImplSDLRenderer2_Init(es->renderer);
    
    editor_init(es);
    
    bool running = true;
    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);            
            if(event.type == SDL_QUIT) {
                running = false;
            }
            if(event.type == SDL_MOUSEWHEEL) {
                es->mouse_wheel = event.wheel.y;
            }
            
        }
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
        
        SDL_SetRenderTarget(es->renderer, back_buffer);
        SDL_SetRenderDrawColor(es->renderer, 180, 200, 180, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(es->renderer);
        editor_update(es);
        editor_render(es);
        SDL_SetRenderTarget(es->renderer, 0);
        ImGui::Image(back_buffer, ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), border_col);
        
        ImGui::End();

        editor_ui(es);

        ImGui::End();
        SDL_SetRenderDrawColor(es->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(es->renderer);
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        SDL_RenderSetScale(es->renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), es->renderer);

        SDL_RenderPresent(es->renderer); 

        es->mouse_wheel = 0;
    }
    editor_shutdown(es);

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(es->renderer);
    SDL_DestroyWindow(es->window);
    SDL_Quit();

    free(es);

    return(0);
};
