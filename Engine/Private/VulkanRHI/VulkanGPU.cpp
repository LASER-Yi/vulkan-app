#include "VulkanRHI/VulkanGPU.h"

#include "Definition.h"
#include "Vulkan/SwapChainSupportDetails.h"

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanRHI/VulkanInstance.h"

#include <cassert>
#include <stdexcept>

const std::vector<const char*> requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

FVulkanGPU::FVulkanGPU(VkPhysicalDevice device, VkSurfaceKHR surface) : device(device), surface(surface), logicalDevice(VK_NULL_HANDLE) {}

FVulkanGPU::~FVulkanGPU()
{
    device = VK_NULL_HANDLE;

    if (IsInit()) {
        vkDestroyDevice(logicalDevice, nullptr);
    }
}

void FVulkanGPU::Init()
{
    assert(device != VK_NULL_HANDLE);

    const QueueFamilyIndices indices = GetQueueFamilies();
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    const std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value()};

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
        // createInfo.enabledLayerCount = validationLayers.size();
        // createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    std::vector<const char*> extensionNames = {};

    // Swapchain
    {
        extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    const VkResult CreateResult =
        vkCreateDevice(device, &createInfo, nullptr, &logicalDevice);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0,
                     &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0,
                     &presentQueue);

    CreateSwapChain();
}

bool FVulkanGPU::IsInit() const
{
    return logicalDevice != VK_NULL_HANDLE;
}

QueueFamilyIndices FVulkanGPU::GetQueueFamilies() const
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

const VkPhysicalDeviceProperties FVulkanGPU::GetProperties() const
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    return std::move(properties);
}

const VkPhysicalDeviceFeatures FVulkanGPU::GetFeatures() const
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    return std::move(features);
}

std::vector<VkExtensionProperties> FVulkanGPU::GetExtensions() const
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         extensions.data());

    return std::move(extensions);
}

bool FVulkanGPU::IsValid() const
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

uint32_t FVulkanGPU::GetScore() const
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

void FVulkanGPU::CreateSwapChain()
{
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

    const QueueFamilyIndices indices = GetQueueFamilies();

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

    const VkResult CreateResult =
        vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }

    // Retrieve swap chain images
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount,
                            swapChainImages.data());

    swapChainImageFormat = createInfo.imageFormat;
    swapChainExtent = createInfo.imageExtent;
}
