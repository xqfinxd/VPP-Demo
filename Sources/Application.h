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
    void CreateSwapchain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateSimplePipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();

    bool IsDeviceSuitable(VkPhysicalDevice gpu) const;
    VkShaderModule CreateShaderModule(const char* filename);
    
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
    
    // concern about device
    VkDevice m_Device = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;

    // concern about swapchain
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    uint32_t m_SwapImageCount = 0;
    std::unique_ptr<VkImage[]> m_SwapImages;
    VkFormat m_SwapImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_SwapImageExtent;
    std::unique_ptr<VkImageView[]> m_SwapImageViews;

    std::unique_ptr<VkFramebuffer[]> m_Framebuffers;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    std::unique_ptr<VkCommandBuffer[]> m_CommandBuffers;

    VkPipeline m_SimplePipeline = VK_NULL_HANDLE;
    VkRenderPass m_SimpleRenderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_SimplePipelineLayout = VK_NULL_HANDLE;
};

