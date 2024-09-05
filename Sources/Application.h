#pragma once

#include <SDL_video.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

class Application {
public:
    struct Queue {
        VkQueue     object = VK_NULL_HANDLE;
        uint32_t    familyIndex = 0;
    };

    struct Swapchain {
        VkSwapchainKHR          object = VK_NULL_HANDLE;
        std::vector<VkImage>    imageObjects;
        VkFormat                imageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D              imageExtent;

        uint32_t GetImageCount() const {
            return (uint32_t)imageObjects.size();
        }
    };

    struct Framebuffer {
        VkFramebuffer   object = VK_NULL_HANDLE;
        VkImageView     colorImage = VK_NULL_HANDLE;
        VkImageView     depthImage = VK_NULL_HANDLE;
        VkExtent2D      extent;
    };

    struct SyncObject {
        VkSemaphore imageAcquired;
        VkSemaphore renderFinished;
        VkFence     inFlight;
    };

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
    void InitImGui();
    void MainLoop();
    void Cleanup();
    bool FrameUpdate();
    void FramePresent();

    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateDevice();
    void RecreateSwapchain();
    void CreateSwapchain();
    void CleanSwapchainResources();

    void CreateSwapchainImpl(VkSwapchainKHR oldSwapchain);
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects(uint32_t frameNum);
    void RecordCommand(uint32_t imageIndex);

    void CreateVertexBuffer(uint32_t stride, uint32_t count, void* data);

    void CreateSimpleRenderPass();

    void CreateSimplePipeline();
    void CreateVertexPipeline();

    bool IsGpuSuitable(VkPhysicalDevice gpu) const;
    VkShaderModule CreateShaderModule(const char* filename);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
private:
    int         m_Width = 0;
    int         m_Height = 0;
    bool        m_Fullscreen = false;
    std::string m_Title;
    uint32_t    m_LockFps = 0;

    bool m_Running = false;
    bool m_WindowResized = false;

    std::vector<const char*> m_ValidationLayers;
    std::vector<const char*> m_DeviceExtensions;

    SDL_Window*         m_Window = nullptr;
    VkInstance          m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR        m_Surface = VK_NULL_HANDLE;
    VkPhysicalDevice    m_Gpu = VK_NULL_HANDLE;
    
    // concern about device
    VkDevice    m_Device = VK_NULL_HANDLE;
    Queue       m_GraphicsQueue;
    Queue       m_PresentQueue;

    // concern about swapchain
    Swapchain   m_Swapchain;

    std::vector<Framebuffer> m_Framebuffers;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    std::unique_ptr<VkCommandBuffer[]> m_CommandBuffers;
    std::unique_ptr<VkCommandBuffer[]> m_ImGuiCommandBuffers;

    VkRenderPass m_SimpleRenderPass = VK_NULL_HANDLE;
    VkRenderPass m_ImGuiRenderPass = VK_NULL_HANDLE;

    VkPipeline m_SimplePipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_SimplePipelineLayout = VK_NULL_HANDLE;

    VkPipeline m_VertexPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_VertexPipelineLayout = VK_NULL_HANDLE;

    uint32_t                m_CurSyncFrame = 0;
    std::vector<SyncObject> m_SyncObjects;

    uint32_t m_FrameIndex = UINT32_MAX;

    VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

    VkDescriptorPool m_ImGuiDescriptorPool = VK_NULL_HANDLE;
};

