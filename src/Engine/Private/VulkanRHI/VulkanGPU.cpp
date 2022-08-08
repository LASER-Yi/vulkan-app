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

FVulkanGpu::FVulkanGpu(VkPhysicalDevice device, VkSurfaceKHR surface,
                       const std::vector<const char*>& layers)
    : device(device), surface(surface), layers(layers)
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

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const VkQueueFamilyProperties& properties : queueFamilies) {
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;

        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        i += 1;
    }

    return indices;
}

const VkPhysicalDeviceProperties FVulkanGpu::GetProperties() const
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    return std::move(properties);
}

const VkPhysicalDeviceFeatures FVulkanGpu::GetFeatures() const
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    return std::move(features);
}

std::vector<VkExtensionProperties> FVulkanGpu::GetExtensions() const
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         extensions.data());

    return std::move(extensions);
}

bool FVulkanGpu::IsValid() const
{
    const auto extensions = GetExtensions();

    std::set<std::string> requiredExts(requiredExtensions.begin(),
                                       requiredExtensions.end());

    for (const VkExtensionProperties& extensionProperties : extensions) {
        requiredExts.erase(extensionProperties.extensionName);
    }

    const bool IsExtensionAvailable = requiredExts.empty();

    const FQueueFamilyIndices indices = GetQueueFamilies();

    return IsExtensionAvailable && indices.isValid();
}

uint32_t FVulkanGpu::GetScore() const
{
    const VkPhysicalDeviceProperties properties = GetProperties();
    const VkPhysicalDeviceFeatures features = GetFeatures();

    uint32_t score = 0;

    switch (properties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        score += 1000;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        score += 100;
        break;
    default:
        break;
    }

    return score;
}

void FVulkanGpu::InitLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const FQueueFamilyIndices indices = GetQueueFamilies();

    const std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(), indices.presentFamily.value()};

    constexpr float queuePriority = 1.0f;
    for (const uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        ZeroVulkanStruct(queueCreateInfo,
                         VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

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
                     [](const VkExtensionProperties& properties) {
                         return strcmp(properties.extensionName,
                                       "VK_KHR_portability_subset");
                     }) != extensions.end()) {
        extensionNames.push_back("VK_KHR_portability_subset");
    }
#endif

    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    VkDevice ld;
    const VkResult CreateResult =
        vkCreateDevice(device, &createInfo, nullptr, &ld);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    logicalDevice = std::make_unique<FVulkanDevice>(ld, this);
}

VkSurfaceKHR FVulkanGpu::GetSurface() const { return surface; }

FVulkanDevice* FVulkanGpu::GetLogicalDevice() const
{
    assert(logicalDevice != nullptr);

    return logicalDevice.get();
}

FSwapChainSupportDetails FVulkanGpu::GetSwapChainSupportDetails() const
{
    return FSwapChainSupportDetails(device, surface);
}
