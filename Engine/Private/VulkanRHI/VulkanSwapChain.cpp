#include "VulkanRHI/VulkanSwapChain.h"
#include <cassert>

FVulkanSwapChain::FVulkanSwapChain(VkSwapchainKHR swapChain)
    : swapChain(swapChain)
{
}

FVulkanSwapChain::~FVulkanSwapChain() { assert(swapChain == VK_NULL_HANDLE); }

void FVulkanSwapChain::Init(VkDevice logicalDevice,
                            const VkSwapchainCreateInfoKHR& CreateInfo)
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
    Images.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount,
                            Images.data());

    ImageFormat = CreateInfo.imageFormat;
    Extent = CreateInfo.imageExtent;
}

void FVulkanSwapChain::Deinit(VkDevice logicalDevice)
{
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
    swapChain = VK_NULL_HANDLE;
}
