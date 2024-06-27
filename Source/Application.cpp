#include "Application.h"

#include <chrono>
#include <thread>
#include <set>
#include <SDL2/SDL.h>

namespace
{

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    void Reset(VkPhysicalDevice device, VkSurfaceKHR surface) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
        }
    }
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.IsComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool CheckDeviceExtension(VkPhysicalDevice device, std::vector<const char*> extensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats,
    VkFormat requiredFormat,
    VkColorSpaceKHR requiredColorSpace) {
    for (const auto& format : availableFormats) {
        if (format.format == requiredFormat &&
            format.colorSpace == requiredColorSpace) {
            return format;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availableModes,
    VkPresentModeKHR requiredMode) {
    for (const auto& mode : availableModes) {
        if (mode == requiredMode) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    int actualWidth, int actualHeight) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {
        VkExtent2D actualExtent{ actualWidth, actualHeight };
        actualExtent.width = SDL_clamp(
            actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);
        actualExtent.height = SDL_clamp(
            actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

}

void Application::Run() {
    InitProperty();
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void Application::InitProperty() {
    m_Width = 1280;
    m_Height = 900;
    m_Fullscreen = false;
    m_Title = "Demo";
    m_LockFps = 60;

    new(&m_ValidationLayers) std::vector<const char*>({
#ifdef _DEBUG
        "VK_LAYER_KHRONOS_validation",
#endif
        });

    new(&m_DeviceExtensions) std::vector<const char*>({
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        });
}

void Application::InitWindow() {
    if (0 != SDL_Init(SDL_INIT_EVERYTHING)) {
        throw std::runtime_error(SDL_GetError());
    }

    uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN;
    if (m_Fullscreen) windowFlags |= SDL_WINDOW_FULLSCREEN;
    m_Window = SDL_CreateWindow(m_Title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_Width, m_Height,
        windowFlags);
    if (!m_Window) {
        throw std::runtime_error(SDL_GetError());
    }
}

void Application::InitVulkan() {
    CreateInstance();
    CreateSurface();
    PickPhysicalDevice();
    CreateDevice();
    CreateSwapchain();
    CreateImageViews();
}

void Application::MainLoop() {
    typedef std::chrono::duration<double, std::micro> Duration;

    auto lastFrameClock = std::chrono::high_resolution_clock::now();

    const Duration k1sec(1'000'000.0);
    Duration duration(0);
    if (m_LockFps)
        duration = k1sec / m_LockFps;

    m_Running = true;
    while (m_Running) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        { // fps
            Duration delta = frameStart - lastFrameClock;
            m_RealtimeFps = k1sec / delta;
            lastFrameClock = frameStart;
        }

        SDL_Event cacheEvent;
        while (SDL_PollEvent(&cacheEvent)) {
            if (SDL_QUIT == cacheEvent.type)
                Exit();
        }

        auto frameEnd = std::chrono::high_resolution_clock::now();
        Duration elapsed = frameEnd - frameStart;
        if (elapsed < duration)
            std::this_thread::sleep_for(duration - elapsed);
    }
}

void Application::Cleanup() {
    for (uint32_t i = 0; i < m_SwapImageCount; i++) {
        vkDestroyImageView(m_Device, m_SwapImageViews[i], nullptr);
        m_SwapImageViews[i] = VK_NULL_HANDLE;
    }
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);

    SDL_DestroyWindow(m_Window);
    SDL_Quit();
}

void Application::CreateInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_Title.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(m_Window, &extensionCount, nullptr);
    std::vector<const char*> extensions(extensionCount);
    SDL_Vulkan_GetInstanceExtensions(m_Window, &extensionCount, extensions.data());

    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.enabledLayerCount = (uint32_t)m_ValidationLayers.size();
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void Application::CreateSurface() {
    if (!SDL_Vulkan_CreateSurface(m_Window, m_Instance, &m_Surface)) {
        throw std::runtime_error(SDL_GetError());
    }
}

void Application::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            m_Gpu = device;
            break;
        }
    }

    if (m_Gpu == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void Application::CreateDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(m_Gpu, m_Surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily,
        indices.presentFamily
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = (uint32_t)m_DeviceExtensions.size();
    createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();
    createInfo.enabledLayerCount = (uint32_t)m_ValidationLayers.size();
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

    if (vkCreateDevice(m_Gpu, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_Device, indices.graphicsFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, indices.presentFamily, 0, &m_PresentQueue);
}

void Application::CreateSwapchain() {
    SwapchainSupportDetails swapchainSupport;
    swapchainSupport.Reset(m_Gpu, m_Surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(
        swapchainSupport.formats,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

    VkPresentModeKHR presentMode = ChooseSwapPresentMode(
        swapchainSupport.presentModes,
        VK_PRESENT_MODE_MAILBOX_KHR);

    int actualWidth = 0, actualHeight = 0;
    SDL_Vulkan_GetDrawableSize(m_Window, &actualWidth, &actualHeight);
    VkExtent2D extent = ChooseSwapExtent(
        swapchainSupport.capabilities,
        actualWidth, actualHeight);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    QueueFamilyIndices indices = FindQueueFamilies(m_Gpu, m_Surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &m_SwapImageCount, nullptr);
    m_SwapImages = std::make_unique<VkImage[]>(m_SwapImageCount);
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &m_SwapImageCount,
        m_SwapImages.get());

    m_SwapImageFormat = surfaceFormat.format;
    m_SwapImageExtent = extent;
}

void Application::CreateImageViews() {
    m_SwapImageViews = std::make_unique<VkImageView[]>(m_SwapImageCount);
    for (size_t i = 0; i < m_SwapImageCount; i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_SwapImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_SwapImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapImageViews[i]) != VK_SUCCESS) {
              throw std::runtime_error("failed to create image views!");
        }
    }
}

bool Application::IsDeviceSuitable(VkPhysicalDevice gpu) const {
    QueueFamilyIndices indices = FindQueueFamilies(gpu, m_Surface);

    bool extensionsSupported = CheckDeviceExtension(gpu, m_DeviceExtensions);

    bool swapchainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapchainSupport;
        swapchainSupport.Reset(gpu, m_Surface);
        swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    }

    return indices.IsComplete() && extensionsSupported && swapchainAdequate;
}

