#include "VulkanRHI/VulkanSwapChain.h"
#include "Definition.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"
#include "VulkanRHI/VulkanGPU.h"

#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

FVulkanSwapChain::FVulkanSwapChain(FVulkanDevice* device)
    : logicalDevice(device), swapChain(), CurrentIndex(INDEX_NONE),
      bSwapchainNeedsResize(false)
{
    CreateSwapChain();
    CreateImageViews();
    CreateFrameBuffers();
    CreateSyncObjects();
}

FVulkanSwapChain::~FVulkanSwapChain()
{
    auto vk_device = logicalDevice->GetDevice();

    vk_device.destroySemaphore(imageAvailableSemaphore);
    vk_device.destroySemaphore(renderFinishedSemaphore);

    Destroy();
}

void FVulkanSwapChain::CreateSwapChain()
{
    auto vk_device = logicalDevice->GetDevice();
    FVulkanGpu* gpu = logicalDevice->GetPhysicalDevice();

    const auto details = gpu->GetSwapChainSupportDetails();
    assert(details.IsValid());

    const vk::SurfaceFormatKHR surfaceFormat =
        details.GetRequiredSurfaceFormat();

    vk::SwapchainCreateInfoKHR createInfo = {
        .sType = vk::StructureType::eSwapchainCreateInfoKHR,
        .surface = details.GetSurface(),
        .minImageCount = details.GetImageCount(),
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = details.GetRequiredExtent(nullptr),
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .preTransform = details.capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = details.GetRequiredPresentMode(),
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr,
    };

    const auto indices = gpu->GetQueueFamilies();

    const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                           indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    VERIFY_VULKAN_RESULT(
        vk_device.createSwapchainKHR(&createInfo, nullptr, &swapChain));

    ImageFormat = createInfo.imageFormat;
    Extent = createInfo.imageExtent;

    Images = vk_device.getSwapchainImagesKHR(swapChain);
}

void FVulkanSwapChain::CreateImageViews()
{
    ImageViews.resize(Images.size());
    auto vk_device = logicalDevice->GetDevice();

    for (size_t i = 0; i < Images.size(); i++) {
        const vk::ImageViewCreateInfo createInfo = {
            .sType = vk::StructureType::eImageViewCreateInfo,
            .image = Images[i],
            .viewType = vk::ImageViewType::e2D,
            .format = ImageFormat,
            .components =
                {
                    .r = vk::ComponentSwizzle::eIdentity,
                    .g = vk::ComponentSwizzle::eIdentity,
                    .b = vk::ComponentSwizzle::eIdentity,
                    .a = vk::ComponentSwizzle::eIdentity,
                },
            .subresourceRange =
                {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        VERIFY_VULKAN_RESULT(
            vk_device.createImageView(&createInfo, nullptr, &ImageViews[i]));
    }
}

void FVulkanSwapChain::CreateFrameBuffers()
{
    const size_t size = Images.size();
    frameBuffers.resize(size);

    auto vk_device = logicalDevice->GetDevice();

    for (size_t i = 0; i < size; i++) {
        const vk::ImageView attachments[] = {ImageViews[i]};

        const vk::FramebufferCreateInfo createInfo = {
            .sType = vk::StructureType::eFramebufferCreateInfo,
            .renderPass = logicalDevice->GetRenderPass(),
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = Extent.width,
            .height = Extent.height,
            .layers = 1,
        };

        VERIFY_VULKAN_RESULT(vk_device.createFramebuffer(&createInfo, nullptr,
                                                         &frameBuffers[i]));
    }
}

vk::Framebuffer FVulkanSwapChain::GetFrameBuffer() const
{
    assert(CurrentIndex < frameBuffers.size() && CurrentIndex != INDEX_NONE);
    return frameBuffers[CurrentIndex];
}

void FVulkanSwapChain::CreateSyncObjects()
{
    const vk::SemaphoreCreateInfo semaphoreInfo = {
        .sType = vk::StructureType::eSemaphoreCreateInfo,
    };

    auto vk_device = logicalDevice->GetDevice();

    VERIFY_VULKAN_RESULT(vk_device.createSemaphore(&semaphoreInfo, nullptr,
                                                   &imageAvailableSemaphore));
    VERIFY_VULKAN_RESULT(vk_device.createSemaphore(&semaphoreInfo, nullptr,
                                                   &renderFinishedSemaphore));
}

void FVulkanSwapChain::Destroy()
{
    auto vk_device = logicalDevice->GetDevice();

    for (auto framebuffer : frameBuffers) {
        vk_device.destroyFramebuffer(framebuffer);
    }
    frameBuffers.clear();

    for (auto imageView : ImageViews) {
        vk_device.destroyImageView(imageView);
    }
    ImageViews.clear();
    Images.clear();

    vk_device.destroySwapchainKHR(swapChain);
    swapChain = nullptr;

    CurrentIndex = INDEX_NONE;
}

void FVulkanSwapChain::AcquireNextImage()
{
    if (bSwapchainNeedsResize) {
        bSwapchainNeedsResize = false;
        Recreate();
    }

    uint32_t nextImageIndex = 0;

    auto vk_device = logicalDevice->GetDevice();

    const vk::Result AcquireResult = vk_device.acquireNextImageKHR(
        swapChain, std::numeric_limits<uint64_t>::max(),
        imageAvailableSemaphore, nullptr, &nextImageIndex);

    switch (AcquireResult) {
    case vk::Result::eErrorOutOfDateKHR:
        Recreate();
        [[clang::fallthrough]];
    case vk::Result::eSuccess:
    case vk::Result::eSuboptimalKHR:
        break;
    default:
        throw std::runtime_error("Failed to acquire next image index");
    }

    CurrentIndex = nextImageIndex;
}

void FVulkanSwapChain::Present()
{
    assert(CurrentIndex != INDEX_NONE);

    const std::vector<vk::Semaphore> waitSemaphores = {
        GetRenderFinishedSemaphore(),
    };

    const std::vector<vk::SwapchainKHR> swapChains = {swapChain};
    const std::vector<uint32_t> swapChainIndices = {
        static_cast<uint32_t>(CurrentIndex),
    };

    const vk::PresentInfoKHR presentInfo = {
        .sType = vk::StructureType::ePresentInfoKHR,
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .swapchainCount = static_cast<uint32_t>(swapChains.size()),
        .pSwapchains = swapChains.data(),
        .pImageIndices = swapChainIndices.data(),
        .pResults = nullptr,
    };

    vk::Queue* graphicsQueue = logicalDevice->GetGraphicsQueue();

    VERIFY_VULKAN_RESULT(graphicsQueue->presentKHR(presentInfo));
}

void FVulkanSwapChain::Recreate()
{
    auto vk_device = logicalDevice->GetDevice();

    vk_device.waitIdle();

    // TODO: Handle minimization (width = 0, height = 0)

    Destroy();

    CreateSwapChain();
    CreateImageViews();
    CreateFrameBuffers();
}

void FVulkanSwapChain::SetNeedResize() { bSwapchainNeedsResize = true; }
