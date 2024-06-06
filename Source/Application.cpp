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
        if (IsDeviceSuitable(device, m_Surface)) {
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

bool Application::IsDeviceSuitable(VkPhysicalDevice gpu, VkSurfaceKHR surface) const {
    QueueFamilyIndices indices = FindQueueFamilies(gpu, surface);

    bool extensionsSupported = CheckDeviceExtension(gpu, m_DeviceExtensions);

    bool swapchainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapChainSupport;
        swapChainSupport.Reset(gpu, surface);
        swapchainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.IsComplete() && extensionsSupported && swapchainAdequate;
}

