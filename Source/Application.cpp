#include "Application.h"

#include <chrono>
#include <thread>
#include <SDL2/SDL.h>

Application::Application(const char* title, int initWidth, int initHeight)
    : m_Title(title), m_Width(initWidth), m_Height(initHeight) {
}

Application::Application(const char* title)
    : m_Title(title), m_Fullscreen(true) {
}

Application::~Application() {
}

void Application::Run() {
    using Duration = std::chrono::duration<double, std::micro>;
    const Duration kOneSec(1'000'000.0);

    auto lastFrameClock = std::chrono::high_resolution_clock::now();

    Duration duration(0);
    if (m_LockFps)
        duration = kOneSec / m_LockFps;

    { // window
        bool succeed = InitWindow();
        SDL_assert(succeed);
    }

    { // vulkan instance
        bool succeed = InitVulkanInstance();
        SDL_assert(succeed);
    }

    m_Running = true;
    while (m_Running) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        { // fps
            Duration delta = frameStart - lastFrameClock;
            m_RealtimeFps = kOneSec / delta;
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

    QuitVulkanInstance();
    QuitWindow();
}

bool Application::InitWindow() {
    if (0 != SDL_Init(SDL_INIT_EVERYTHING)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, SDL_GetError());
        return false;
    }

    uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN;
    if (m_Fullscreen) windowFlags |= SDL_WINDOW_FULLSCREEN;
    m_Window = SDL_CreateWindow(m_Title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_Width, m_Height,
        windowFlags);
    if (!m_Window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, SDL_GetError());
        return false;
    }

    return true;
}

void Application::QuitWindow() {
    if (m_Window) {
        SDL_DestroyWindow(m_Window);
        m_Window = NULL;
    }
}

bool Application::InitVulkanInstance() {
    auto appCI = vk::ApplicationInfo()
        .setPNext(nullptr)
        .setPApplicationName("VPP-Demo")
        .setApplicationVersion(0)
        .setApiVersion(VK_API_VERSION_1_1)
        .setPEngineName("None")
        .setEngineVersion(0);

    std::vector<const char*> extensions;
    uint32_t extensionCount = 0;
    if (SDL_Vulkan_GetInstanceExtensions(m_Window, &extensionCount, nullptr) ==
        SDL_TRUE) {
        extensions.resize(extensionCount);
        SDL_Vulkan_GetInstanceExtensions(m_Window, &extensionCount,
            extensions.data());
    }
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    std::vector<const char*> layers{
        "VK_LAYER_KHRONOS_validation",
    };

    auto instCI = vk::InstanceCreateInfo()
        .setPEnabledLayerNames(layers)
        .setPEnabledExtensionNames(extensions)
        .setPApplicationInfo(&appCI);

    auto result = vk::createInstance(&instCI, nullptr, &m_VulkanInstance);
    return result == vk::Result::eSuccess;
}

void Application::QuitVulkanInstance() {
    if (m_VulkanInstance) {
        m_VulkanInstance.destroy();
    }
}

