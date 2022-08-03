#include "VulkanRHI/VulkanSwapChain.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"
#include <cassert>
#include <stdexcept>

FVulkanSwapChain::FVulkanSwapChain(VkSwapchainKHR swapChain,
                                   FVulkanDevice* device, VkFormat format,
                                   VkExtent2D extent)
    : swapChain(swapChain), logicalDevice(device), ImageFormat(format),
      Extent(extent)
{
    VkDevice _device = logicalDevice->GetDevice();

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(_device, swapChain, &imageCount, nullptr);
    Images.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, swapChain, &imageCount, Images.data());

    CreateImageViews();
}

FVulkanSwapChain::~FVulkanSwapChain()
{
    VkDevice _device = logicalDevice->GetDevice();

    for (auto imageView : ImageViews) {
        vkDestroyImageView(_device, imageView, nullptr);
    }
    ImageViews.clear();
    Images.clear();

    vkDestroySwapchainKHR(_device, swapChain, nullptr);
    swapChain = VK_NULL_HANDLE;
}

void FVulkanSwapChain::CreateImageViews()
{
    ImageViews.resize(Images.size());
    VkDevice _device = logicalDevice->GetDevice();

    for (size_t i = 0; i < Images.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
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

        const VkResult CreateResult =
            vkCreateImageView(_device, &createInfo, nullptr, &ImageViews[i]);

        if (CreateResult != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view");
        }
    }
}
