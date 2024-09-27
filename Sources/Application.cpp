#include "Application.h"
#include <iostream>
#include <SDL.h>

IApplication* IApplication::Create() {
    return new Application();
}

Application::Application() :
    IApplication(),
    m_Window(nullptr) {
}

Application::~Application() {
}

void Application::Initialize() {
    if (0 != SDL_Init(SDL_INIT_EVERYTHING)) {
        throw std::runtime_error(SDL_GetError());
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN;
    switch (m_DisplayMode) {
    case DisplayMode::eFullscreen:
        windowFlags |= SDL_WINDOW_FULLSCREEN;
        break;
    case DisplayMode::eFullscreenWindowed:
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        break;
    default:
        break;
    }

    m_Window = SDL_CreateWindow(m_Title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_Width, m_Height,
        windowFlags);
    if (!m_Window) {
        throw std::runtime_error(SDL_GetError());
    }
}

void Application::FrameRun() {
    SDL_Event cacheEvent;
    while (SDL_PollEvent(&cacheEvent)) {
        if (SDL_QUIT == cacheEvent.type) {
            Exit();
        }
        if (SDL_KEYUP == cacheEvent.type) {
            const auto& keyinfo = cacheEvent.key;
            if (keyinfo.keysym.sym == SDLK_ESCAPE) {
                Exit();
            }
        }

        // ImGui_ImplSDL2_ProcessEvent(&cacheEvent);
    }
}

void Application::Finalize() {
    if (m_Window)
        SDL_DestroyWindow(m_Window);
    SDL_Quit();
}
