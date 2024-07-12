#include <stdio.h>
#include <filesystem>
#include <string>

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
#include "entity.h"
#include "editor.h"
// Srouces
#include "input.cpp"
#include "entity.cpp"
#include "utils.cpp"
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
    
    // and allocate the editor state
    EditorState *es = (EditorState *)malloc(sizeof(EditorState));

    SDL_Init(SDL_INIT_EVERYTHING);
    es->window = SDL_CreateWindow("EDF Editor",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                WINDOW_WIDTH, WINDOW_HEIGHT, 
                                SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN|SDL_WINDOW_MAXIMIZED);
    es->renderer = SDL_CreateRenderer(es->window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    es->back_buffer = SDL_CreateTexture(es->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                                 BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT);
    es->mouse_picking_buffer = SDL_CreateTexture(es->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                                 BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT);

    u32 format;
    i32 w, h;
    SDL_QueryTexture(es->mouse_picking_buffer, &format, 0, &w, &h);
    i32 bytes_per_pixel = SDL_BYTESPERPIXEL(format);
    es->mouse_picking_pixels = (u32 *)malloc(bytes_per_pixel * w * h);


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    
    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
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

            if(event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                es->just_focus = true;
                g_input[0] = {};
                g_input[1] = {};
            }
            if(event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                g_input[0] = {};
                g_input[1] = {};
            }

            if(event.type == SDL_MOUSEWHEEL) {
                es->mouse_wheel = event.wheel.y;
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

        SDL_GetMouseState(&g_input[0].mouse_x, &g_input[0].mouse_y);

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

        ImGuiStyle &style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(0, 0);
        static bool condition = true;
        ImGui::Begin("Dockspace demo", &condition, windowFlags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::DockSpace(ImGui::GetID("Dockspace"));

        // draw the back buffer

        ImGui::Begin("Game Viewport", 0, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);

        SDL_SetRenderTarget(es->renderer, es->mouse_picking_buffer);
        SDL_SetRenderDrawColor(es->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(es->renderer);
        Entity *entity = es->em.first;
        while(entity) {
            u8 r = (entity->uid >> 16) & 0xFF;
            u8 g = (entity->uid >>  8) & 0xFF;
            u8 b = (entity->uid >>  0) & 0xFF;
            SDL_SetTextureColorMod(entity->texture.mask, r, g, b);
            draw_quad(es, 
                      entity->pos.x, entity->pos.y,
                      entity->scale.x, entity->scale.y,
                      entity->texture.mask);
            entity = entity->next;
        }

        
        
        SDL_SetRenderTarget(es->renderer, es->back_buffer);
        SDL_SetRenderDrawColor(es->renderer, 25, 25, 25, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(es->renderer);
        if(!es->just_focus) {
            editor_update(es);
        }
        editor_render(es);
        SDL_SetRenderTarget(es->renderer, 0);
        ImGui::Image(es->back_buffer, ImVec2(BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
        
        ImGui::End();
        style.WindowPadding = ImVec2(8, 8);

        editor_ui(es);


        style.WindowPadding = ImVec2(0, 0);
        ImGui::End();
        SDL_SetRenderDrawColor(es->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(es->renderer);
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        SDL_RenderSetScale(es->renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), es->renderer);

        SDL_RenderPresent(es->renderer); 

        // swap the input pointer
        g_input[1] = g_input[0];
        es->mouse_wheel = 0;
        es->just_focus = false;
    }
    editor_shutdown(es);

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    free(es->mouse_picking_pixels);
    SDL_DestroyTexture(es->mouse_picking_buffer);
    SDL_DestroyTexture(es->back_buffer);

    SDL_DestroyRenderer(es->renderer);
    SDL_DestroyWindow(es->window);
    SDL_Quit();

    free(es);

    return(0);
};
