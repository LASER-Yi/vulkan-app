#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include <vector>
#include <vulkan/vulkan_core.h>

class FVulkanDevice;

class FVulkanSwapChain
{
  public:
    FVulkanSwapChain(VkSwapchainKHR swapChain, FVulkanDevice* device,
                     VkFormat format, VkExtent2D extent);
    ~FVulkanSwapChain();

    VkFormat GetFormat() const { return ImageFormat; }
    VkExtent2D GetExtent() const { return Extent; }

    void CreateFrameBuffers();
    VkFramebuffer GetFrameBuffer() const;

    uint32_t AcquireNextImage();
    uint32_t GetCurrentImage() const { return cachedNextImage; };

    VkSemaphore GetImageAvailableSemaphore() const
    {
        return imageAvailableSemaphore;
    }

    VkSemaphore GetRenderFinishedSemaphore() const
    {
        return renderFinishedSemaphore;
    }

    void Present();

  protected:
    FVulkanDevice* logicalDevice;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> Images;
    std::vector<VkImageView> ImageViews;
    VkFormat ImageFormat;
    VkExtent2D Extent;

    std::vector<VkFramebuffer> frameBuffers;

    uint32_t cachedNextImage;

    // sync objects
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

  private:
    void CreateImageViews();
    void CreateSyncObjects();
};
