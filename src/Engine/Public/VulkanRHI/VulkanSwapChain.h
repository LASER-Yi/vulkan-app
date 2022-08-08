#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

#include <vector>

class FVulkanDevice;

class FVulkanSwapChain
{
  public:
    FVulkanSwapChain(FVulkanDevice* device);
    FVulkanSwapChain(const FVulkanSwapChain& other) = delete;
    ~FVulkanSwapChain();

    vk::Format GetFormat() const { return ImageFormat; }
    vk::Extent2D GetExtent() const { return Extent; }

    vk::Framebuffer GetFrameBuffer() const;

    uint32_t AcquireNextImage();
    uint32_t GetCurrentImage() const { return cachedNextImage; };

    vk::Semaphore GetImageAvailableSemaphore() const
    {
        return imageAvailableSemaphore;
    }

    vk::Semaphore GetRenderFinishedSemaphore() const
    {
        return renderFinishedSemaphore;
    }

    void Present();

    void Recreate();

    void SetNeedResize();

  protected:
    FVulkanDevice* logicalDevice;

    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> Images;
    std::vector<vk::ImageView> ImageViews;
    vk::Format ImageFormat;
    vk::Extent2D Extent;

    std::vector<vk::Framebuffer> frameBuffers;

    uint32_t cachedNextImage;

    bool bSwapchainNeedsResize;

    // sync objects
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;

  private:
    void CreateSwapChain();
    void CreateImageViews();
    void CreateFrameBuffers();
    void CreateSyncObjects();

    void Destroy();
};
