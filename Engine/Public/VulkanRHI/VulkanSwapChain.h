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
    VkFramebuffer GetFrameBuffer(uint32_t index) const;

    uint32_t GetNextImageIndex() const;

    VkSemaphore GetImageAvailableSemaphore() const
    {
        return imageAvailableSemaphore;
    }

    VkSemaphore GetRenderFinishedSemaphore() const
    {
        return renderFinishedSemaphore;
    }

    VkSwapchainKHR TEMP_GetSwapChain() const { return swapChain; }

  protected:
    FVulkanDevice* logicalDevice;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> Images;
    std::vector<VkImageView> ImageViews;
    VkFormat ImageFormat;
    VkExtent2D Extent;

    std::vector<VkFramebuffer> frameBuffers;

    // sync objects
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

  private:
    void CreateImageViews();
    void CreateSyncObjects();
};
