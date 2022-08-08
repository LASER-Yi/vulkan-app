#include "VulkanRHI/VulkanSwapChain.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"
#include "VulkanRHI/VulkanGPU.h"

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <vector>

FVulkanSwapChain::FVulkanSwapChain(FVulkanDevice* device)
    : logicalDevice(device), swapChain(VK_NULL_HANDLE), cachedNextImage(0),
      bSwapchainNeedsResize(false)
{
    VkDevice _device = logicalDevice->GetDevice();

    CreateSwapChain();
    CreateImageViews();
    CreateFrameBuffers();
    CreateSyncObjects();
}

FVulkanSwapChain::~FVulkanSwapChain()
{
    VkDevice _device = logicalDevice->GetDevice();

    vkDestroySemaphore(_device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(_device, renderFinishedSemaphore, nullptr);

    Destroy();
}

void FVulkanSwapChain::CreateSwapChain()
{
    VkDevice _device = logicalDevice->GetDevice();
    FVulkanGpu* gpu = logicalDevice->GetPhysicalDevice();

    const auto details = gpu->GetSwapChainSupportDetails();
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

    const auto indices = gpu->GetQueueFamilies();

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
        vkCreateSwapchainKHR(_device, &createInfo, nullptr, &swapChain);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }
    ImageFormat = createInfo.imageFormat;
    Extent = createInfo.imageExtent;

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(_device, swapChain, &imageCount, nullptr);
    Images.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, swapChain, &imageCount, Images.data());
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

VkFramebuffer FVulkanSwapChain::GetFrameBuffer() const
{
    assert(cachedNextImage < frameBuffers.size());
    return frameBuffers[cachedNextImage];
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

void FVulkanSwapChain::Destroy()
{
    VkDevice _device = logicalDevice->GetDevice();

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

uint32_t FVulkanSwapChain::AcquireNextImage()
{
    if (bSwapchainNeedsResize) {
        bSwapchainNeedsResize = false;
        Recreate();
    }

    uint32_t nextImageIndex = 0;
    const VkResult AcquireResult = vkAcquireNextImageKHR(
        logicalDevice->GetDevice(), swapChain,
        std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore,
        VK_NULL_HANDLE, &nextImageIndex);

    switch (AcquireResult) {
    case VK_ERROR_OUT_OF_DATE_KHR:
        Recreate();
        [[clang::fallthrough]];
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR:
        break;
    default:
        throw std::runtime_error("Failed to acquire next image index");
    }

    cachedNextImage = nextImageIndex;

    return nextImageIndex;
}

void FVulkanSwapChain::Present()
{
    VkPresentInfoKHR presentInfo = {};
    ZeroVulkanStruct(presentInfo, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    const std::vector<VkSemaphore> waitSemaphores = {
        GetRenderFinishedSemaphore()};

    presentInfo.waitSemaphoreCount = waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();

    const VkSwapchainKHR swapChains[] = {swapChain};

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &cachedNextImage;

    presentInfo.pResults = nullptr;

    VkQueue graphicsQueue = logicalDevice->GetGraphicsQueue();

    vkQueuePresentKHR(graphicsQueue, &presentInfo);
}

void FVulkanSwapChain::Recreate()
{
    VkDevice vk_device = logicalDevice->GetDevice();

    vkDeviceWaitIdle(vk_device);

    // TODO: Handle minimization (width = 0, height = 0)

    Destroy();

    CreateSwapChain();
    CreateImageViews();
    CreateFrameBuffers();
}

void FVulkanSwapChain::SetNeedResize() { bSwapchainNeedsResize = true; }
