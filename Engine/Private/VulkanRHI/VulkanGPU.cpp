#include "VulkanRHI/VulkanGPU.h"

#include "Definition.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanRHI/SwapChainSupportDetails.h"
#include "VulkanRHI/VulkanInstance.h"
#include "VulkanRHI/VulkanSwapChain.h"

const std::vector<const char*> requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

FVulkanGPUCreateParam::FVulkanGPUCreateParam() {}

QueueFamilyIndices FVulkanGPUCreateParam::GetQueueFamilies() const
{
    QueueFamilyIndices indices;

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

const VkPhysicalDeviceProperties FVulkanGPUCreateParam::GetProperties() const
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    return std::move(properties);
}

const VkPhysicalDeviceFeatures FVulkanGPUCreateParam::GetFeatures() const
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    return std::move(features);
}

std::vector<VkExtensionProperties> FVulkanGPUCreateParam::GetExtensions() const
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         extensions.data());

    return std::move(extensions);
}

bool FVulkanGPUCreateParam::IsValid() const
{
    const auto extensions = GetExtensions();

    std::set<std::string> requiredExts(requiredExtensions.begin(),
                                       requiredExtensions.end());

    for (const VkExtensionProperties& extensionProperties : extensions) {
        requiredExts.erase(extensionProperties.extensionName);
    }

    const bool IsExtensionAvailable = requiredExts.empty();

    const QueueFamilyIndices indices = GetQueueFamilies();

    return IsExtensionAvailable && indices.isValid();
}

uint32_t FVulkanGPUCreateParam::GetScore() const
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

// BEGIN: Vulkan GPU
FVulkanGPU::FVulkanGPU(const FVulkanGPUCreateParam& Param) { Init(Param); }

FVulkanGPU::~FVulkanGPU()
{
    swapChain->Deinit(logicalDevice);
    swapChain.reset();

    vkDestroyDevice(logicalDevice, nullptr);
    device = VK_NULL_HANDLE;
}

void FVulkanGPU::Init(const FVulkanGPUCreateParam& Param)
{
    assert(Param.IsValid());

    device = Param.device;
    indices = Param.GetQueueFamilies();

    CreateLogicalDevice(Param);
    CreateSwapChain(Param);

    CreateDeviceQueue();
}

void FVulkanGPU::CreateLogicalDevice(const FVulkanGPUCreateParam& Param)
{
    assert(Param.IsValid());

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    const std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(), indices.presentFamily.value()};

    constexpr float queuePriority = 1.0f;
    for (const uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    if (GE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = Param.layers.size();
        createInfo.ppEnabledLayerNames = Param.layers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    std::vector<const char*> extensionNames = {};

    // Swapchain
    extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    const VkResult CreateResult =
        vkCreateDevice(device, &createInfo, nullptr, &logicalDevice);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }
}

void FVulkanGPU::CreateSwapChain(const FVulkanGPUCreateParam& Param)
{
    assert(Param.IsValid());
    VkSurfaceKHR surface = Param.surface;

    const SwapChainSupportDetails details(device, surface);

    assert(details.IsValid());

    const VkSurfaceFormatKHR surfaceFormat = details.GetRequiredSurfaceFormat();

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = details.GetImageCount();

    createInfo.presentMode = details.GetRequiredPresentMode();
    createInfo.clipped = VK_TRUE;

    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;

    // TODO: Better struct
    createInfo.imageExtent = details.GetRequiredExtent(nullptr);

    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                           indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR sc;

    const VkResult CreateResult =
        vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &sc);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }

    swapChain = std::make_shared<FVulkanSwapChain>(sc);
    swapChain->Init(logicalDevice, createInfo);
}

void FVulkanGPU::CreateDeviceQueue()
{
    assert(logicalDevice != VK_NULL_HANDLE);

    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0,
                     &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0,
                     &presentQueue);
}
