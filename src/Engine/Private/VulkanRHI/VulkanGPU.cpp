#include "VulkanRHI/VulkanGPU.h"

#include "Definition.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/SwapChainSupportDetails.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"
#include "VulkanRHI/VulkanInstance.h"
#include "VulkanRHI/VulkanSwapChain.h"

const std::vector<const char*> requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

FVulkanGpu::FVulkanGpu(vk::PhysicalDevice device, FVulkanInstance* instance,
                       const std::vector<const char*>& layers)
    : device(device), instance(instance), layers(layers)
{
}

FVulkanGpu::~FVulkanGpu()
{
    logicalDevice.reset();

    device = VK_NULL_HANDLE;
}

FQueueFamilyIndices FVulkanGpu::GetQueueFamilies() const
{
    FQueueFamilyIndices indices;

    const auto queueFamilies = device.getQueueFamilyProperties();

    vk::SurfaceKHR surface = instance->GetSurface();

    int i = 0;
    for (const vk::QueueFamilyProperties& properties : queueFamilies) {
        if (properties.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;

        VERIFY_VULKAN_RESULT(
            device.getSurfaceSupportKHR(i, surface, &presentSupport));

        if (presentSupport) {
            indices.presentFamily = i;
        }

        i += 1;
    }

    return indices;
}

const vk::PhysicalDeviceProperties FVulkanGpu::GetProperties() const
{
    return device.getProperties();
}

const vk::PhysicalDeviceFeatures FVulkanGpu::GetFeatures() const
{
    return device.getFeatures();
}

std::vector<vk::ExtensionProperties> FVulkanGpu::GetExtensions() const
{
    return device.enumerateDeviceExtensionProperties();
}

bool FVulkanGpu::IsValid() const
{
    const auto extensions = GetExtensions();

    std::set<std::string> requiredExts(requiredExtensions.begin(),
                                       requiredExtensions.end());

    for (const auto& extensionProperties : extensions) {
        requiredExts.erase(extensionProperties.extensionName);
    }

    const bool IsExtensionAvailable = requiredExts.empty();

    const FQueueFamilyIndices indices = GetQueueFamilies();

    return IsExtensionAvailable && indices.isValid();
}

uint32_t FVulkanGpu::GetScore() const
{
    const auto properties = GetProperties();
    const auto features = GetFeatures();

    uint32_t score = 0;

    switch (properties.deviceType) {
    case vk::PhysicalDeviceType::eDiscreteGpu:
        score += 1000;
        break;
    case vk::PhysicalDeviceType::eIntegratedGpu:
        score += 100;
        break;
    default:
        break;
    }

    return score;
}

void FVulkanGpu::InitLogicalDevice()
{
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    const FQueueFamilyIndices indices = GetQueueFamilies();

    const std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(), indices.presentFamily.value()};

    constexpr float queuePriority = 1.0f;
    for (const uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo = {
            .sType = vk::StructureType::eDeviceQueueCreateInfo,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };

        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures = {};

    vk::DeviceCreateInfo createInfo = {
        .sType = vk::StructureType::eDeviceCreateInfo,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    if (GE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = layers.size();
        createInfo.ppEnabledLayerNames = layers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    std::vector<const char*> extensionNames = {};

    // Swapchain
    extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#if PLATFORM_APPLE
    const auto extensions = GetExtensions();

    if (std::find_if(extensions.begin(), extensions.end(),
                     [](const vk::ExtensionProperties& properties) {
                         return strcmp(properties.extensionName,
                                       "VK_KHR_portability_subset");
                     }) != extensions.end()) {
        extensionNames.push_back("VK_KHR_portability_subset");
    }
#endif

    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    vk::Device vk_device;
    VERIFY_VULKAN_RESULT(device.createDevice(&createInfo, nullptr, &vk_device));

    logicalDevice = std::make_unique<FVulkanDevice>(vk_device, this);
}

FVulkanDevice* FVulkanGpu::GetLogicalDevice() const
{
    assert(logicalDevice != nullptr);

    return logicalDevice.get();
}

FSwapChainSupportDetails FVulkanGpu::GetSwapChainSupportDetails() const
{
    vk::SurfaceKHR surface = instance->GetSurface();
    return FSwapChainSupportDetails(device, surface);
}
