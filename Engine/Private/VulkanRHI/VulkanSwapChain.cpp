#include "VulkanRHI/VulkanSwapChain.h"
#include <cassert>
#include <stdexcept>

FVulkanSwapChain::FVulkanSwapChain(VkSwapchainKHR swapChain)
    : swapChain(swapChain)
{
}

FVulkanSwapChain::~FVulkanSwapChain()
{
    assert(swapChain == VK_NULL_HANDLE);
    assert(ImageViews.size() == 0);
}

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

    CreateImageViews(logicalDevice);
}

void FVulkanSwapChain::Deinit(VkDevice logicalDevice)
{
    for (auto imageView : ImageViews) {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }
    ImageViews.clear();
    Images.clear();

    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
    swapChain = VK_NULL_HANDLE;
}

void FVulkanSwapChain::CreateImageViews(VkDevice logicalDevice)
{
    ImageViews.resize(Images.size());

    for (size_t i = 0; i < Images.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = Images[i];

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = ImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        const VkResult CreateResult = vkCreateImageView(
            logicalDevice, &createInfo, nullptr, &ImageViews[i]);

        if (CreateResult != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view");
        }
    }
}
