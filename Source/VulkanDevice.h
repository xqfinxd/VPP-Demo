#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class Application;

class VulkanDevice {
public:
    VulkanDevice(const Application& app);
    ~VulkanDevice();

private:
    std::vector<vk::PhysicalDevice> m_Gpus;
    uint32_t m_GpuIndex = UINT32_MAX;
    std::vector<vk::QueueFamilyProperties> m_QueueFamilyProps;
    uint32_t m_QueueFamilyIndex = UINT32_MAX;
    vk::PhysicalDeviceFeatures m_EnabledFeatures;
    vk::Device m_Device = nullptr;
    vk::Queue m_Queue = nullptr;
};

