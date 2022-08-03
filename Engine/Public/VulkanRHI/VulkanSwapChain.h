#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include <vector>

class FVulkanDevice;

class FVulkanSwapChain
{
  public:
    FVulkanSwapChain(VkSwapchainKHR swapChain, FVulkanDevice* device,
                     VkFormat format, VkExtent2D extent);
    ~FVulkanSwapChain();

  protected:
    FVulkanDevice* logicalDevice;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> Images;
    std::vector<VkImageView> ImageViews;
    VkFormat ImageFormat;
    VkExtent2D Extent;

  private:
    void CreateImageViews();
};
