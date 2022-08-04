#include "VulkanRHI/VulkanSwapChain.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"
#include <cassert>
#include <cstddef>
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
    CreateSyncObjects();
}

FVulkanSwapChain::~FVulkanSwapChain()
{
    VkDevice _device = logicalDevice->GetDevice();

    vkDestroySemaphore(_device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(_device, renderFinishedSemaphore, nullptr);

    for (auto framebuffer : frameBuffers) {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }
    frameBuffers.clear();

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

void FVulkanSwapChain::CreateFrameBuffers()
{
    const size_t size = Images.size();
    frameBuffers.resize(size);

    for (size_t i = 0; i < size; i++) {
        VkImageView attachments[] = {ImageViews[i]};

        VkFramebufferCreateInfo createInfo = {};
        ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        createInfo.renderPass = logicalDevice->GetRenderPass();
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = attachments;
        createInfo.width = Extent.width;
        createInfo.height = Extent.height;
        createInfo.layers = 1;

        const VkResult CreateResult = vkCreateFramebuffer(
            logicalDevice->GetDevice(), &createInfo, nullptr, &frameBuffers[i]);

        if (CreateResult != VK_SUCCESS) {
            throw std::runtime_error("Failed to create frame buffer");
        }
    }
}

VkFramebuffer FVulkanSwapChain::GetFrameBuffer(uint32_t index) const
{
    assert(index < frameBuffers.size());
    return frameBuffers[index];
}

void FVulkanSwapChain::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    ZeroVulkanStruct(semaphoreInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkDevice _device = logicalDevice->GetDevice();
    if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr,
                          &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(_device, &semaphoreInfo, nullptr,
                          &renderFinishedSemaphore) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphores!");
    }
}

uint32_t FVulkanSwapChain::GetNextImageIndex() const
{
    uint32_t nextImageIndex = 0;
    vkAcquireNextImageKHR(logicalDevice->GetDevice(), swapChain,
                          std::numeric_limits<uint64_t>::max(),
                          imageAvailableSemaphore, VK_NULL_HANDLE,
                          &nextImageIndex);

    return nextImageIndex;
}
