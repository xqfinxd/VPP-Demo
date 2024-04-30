#pragma once

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

class Application {
public:
    Application(const char* title, int initWidth, int initHeight);
    Application(const char* title);
    ~Application();

    void SetLockFps(uint32_t fps) {
        m_LockFps = fps;
    }

    void Run();
    void Exit() {
        m_Running = false;
    }

protected:
    bool InitWindow();
    void QuitWindow();

    bool InitVulkanInstance();
    void QuitVulkanInstance();

private:
    int m_Width = 0;
    int m_Height = 0;
    bool m_Fullscreen = false;
    std::string m_Title;

    uint32_t m_LockFps = 0;
    double m_RealtimeFps = 0;
    bool m_Running = false;

    SDL_Window* m_Window = nullptr;
    vk::Instance m_VulkanInstance = nullptr;
};

