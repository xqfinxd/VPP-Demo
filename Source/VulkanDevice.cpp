#include "VulkanDevice.h"
#include "Application.h"

namespace {

struct GpuEvaluate {
    const vk::PhysicalDevice& gpu_;
    const vk::SurfaceKHR& surf_;

    GpuEvaluate(const vk::PhysicalDevice& gpu, const vk::SurfaceKHR& surf)
        : gpu_(gpu), surf_(surf) { }

    bool operator()(const vk::QueueFamilyProperties& queueProp, uint32_t index) {
        auto properties = gpu_.getProperties();
        if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) {
            return false;
        }

        vk::QueueFlags desiredFlags(
            vk::QueueFlagBits::eGraphics |
            vk::QueueFlagBits::eTransfer);
        if (!(queueProp.queueFlags & desiredFlags)) {
            return false;
        }

        if (!gpu_.getSurfaceSupportKHR(index, surf_)) {
            return false;
        }

        return true;
    }
};

}

VulkanDevice::VulkanDevice(const Application& app) {
    m_Gpus = app.m_VulkanInstance.enumeratePhysicalDevices();
    for (uint32_t i = 0; i < m_Gpus.size(); i++) {
        const auto& gpu = m_Gpus[i];

        GpuEvaluate evalute(gpu, app.m_WindowSurface);
        bool found = false;
        auto queueFamilyProps = gpu.getQueueFamilyProperties();
        for (uint32_t qi = 0; qi < queueFamilyProps.size(); qi++) {
            if (evalute(queueFamilyProps[qi], qi)) {
                found = true;
                m_GpuIndex = i;
                m_QueueFamilyProps = queueFamilyProps;
                m_QueueFamilyIndex = qi;
                break;
            }
        }

        if (found)
            break;
    }
    
    if (m_GpuIndex >= m_Gpus.size() ||
        m_QueueFamilyIndex >= m_QueueFamilyProps.size()) {
        throw std::runtime_error("no available physical devices found!");
    }

    float queuePriority = 1.0f;
    auto queueCI = vk::DeviceQueueCreateInfo()
        .setPQueuePriorities(&queuePriority)
        .setQueueFamilyIndex(m_QueueFamilyIndex)
        .setQueueCount(1);

    std::vector<const char*> layers{
        "VK_LAYER_KHRONOS_validation",
    };
    std::vector<const char*> extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    auto deviceCI = vk::DeviceCreateInfo()
        .setQueueCreateInfoCount(1)
        .setPQueueCreateInfos(&queueCI)
        .setPEnabledFeatures(&m_EnabledFeatures)
        .setPEnabledLayerNames(layers)
        .setPEnabledExtensionNames(extensions);
    m_Device = m_Gpus[m_GpuIndex].createDevice(deviceCI);
    if (!m_Device) {
        throw std::runtime_error("failed to create device!");
    }
    m_Queue = m_Device.getQueue(m_QueueFamilyIndex, 0);
}

VulkanDevice::~VulkanDevice() {
    if (m_Device) {
        m_Device.destroy();
    }
}
