#pragma once

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

struct QueueFamilyIndices {
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;

    bool IsComplete() const {
        return graphicsFamily != UINT32_MAX
            && presentFamily != UINT32_MAX;
    }
};

class Application {
    friend class VulkanDevice;
public:
    void SetLockFps(uint32_t fps) {
        m_LockFps = fps;
    }

    void Run();
    void Exit() {
        m_Running = false;
    }

private:
    void InitProperty();
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateDevice();

    bool IsDeviceSuitable(VkPhysicalDevice gpu, VkSurfaceKHR surface) const;
    

private:
    int m_Width = 0;
    int m_Height = 0;
    bool m_Fullscreen = false;
    std::string m_Title;
    uint32_t m_LockFps = 0;

    double m_RealtimeFps = 0;
    bool m_Running = false;

    std::vector<const char*> m_ValidationLayers;
    std::vector<const char*> m_DeviceExtensions;

    SDL_Window* m_Window = nullptr;
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_Gpu = VK_NULL_HANDLE;
    VkDevice m_Device;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
};

