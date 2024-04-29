#pragma once

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

class Application
{
public:
    Application(const char* title, int initWidth, int initHeight);
    Application(const char* title);
    ~Application();

    void ShowWindow(bool show)
    {
        m_WindowShown = show;
    }

private:
    int m_Width = 0;
    int m_Height = 0;
    bool m_Fullscreen = false;
    std::string m_Title;

    bool m_WindowShown = false;

    SDL_Window* m_Window = nullptr;
    vk::Instance m_VulkanInstance = nullptr;
};

