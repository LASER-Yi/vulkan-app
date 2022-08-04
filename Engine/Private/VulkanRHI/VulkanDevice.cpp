#include "VulkanRHI/VulkanDevice.h"

#include "Core/FileManager.h"
#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanShader.h"
#include "VulkanRHI/VulkanSwapChain.h"

#include <cassert>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

FVulkanDevice::FVulkanDevice(VkDevice device, const QueueFamilyIndices& indices)
    : device(device), indices(indices)
{
    InitDeviceQueue();
}

FVulkanDevice::~FVulkanDevice()
{
    shaders.clear();

    swapChain.reset();

    vkDestroyDevice(device, nullptr);
    device = VK_NULL_HANDLE;
}

std::shared_ptr<FVulkanShader>
FVulkanDevice::CreateShader(const std::string& filename,
                            VkShaderStageFlagBits stage)
{
    const FileBlob blob = FileManager::ReadFile(filename);

    auto _shader = std::make_shared<FVulkanShader>(this, blob, stage, "main");

    shaders.push_back(_shader);

    return _shader;
}

void FVulkanDevice::InitSwapChain(const SwapChainSupportDetails& details)
{
    assert(device != VK_NULL_HANDLE);
    assert(details.IsValid());

    const VkSurfaceFormatKHR surfaceFormat = details.GetRequiredSurfaceFormat();

    VkSwapchainCreateInfoKHR createInfo = {};
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = details.GetSurface();
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
        vkCreateSwapchainKHR(device, &createInfo, nullptr, &sc);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }

    swapChain = std::make_unique<FVulkanSwapChain>(
        sc, this, createInfo.imageFormat, createInfo.imageExtent);
}

void FVulkanDevice::InitDeviceQueue()
{
    assert(device != VK_NULL_HANDLE);

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}
