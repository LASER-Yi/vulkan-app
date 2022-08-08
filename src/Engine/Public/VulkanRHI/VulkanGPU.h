#pragma once

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/SwapChainSupportDetails.h"
#include "VulkanRHI/VulkanDevice.h"
#include "VulkanRHI/VulkanShader.h"

class FVulkanInstance;

// Physical Device Wrapper
class FVulkanGpu
{
  public:
    FVulkanGpu(VkPhysicalDevice device, VkSurfaceKHR surface,
               const std::vector<const char*>& layers);
    ~FVulkanGpu();

    void InitLogicalDevice();

    FQueueFamilyIndices GetQueueFamilies() const;
    const VkPhysicalDeviceProperties GetProperties() const;
    const VkPhysicalDeviceFeatures GetFeatures() const;

    std::vector<VkExtensionProperties> GetExtensions() const;

    FSwapChainSupportDetails GetSwapChainSupportDetails() const;

    bool IsValid() const;
    uint32_t GetScore() const;

    VkSurfaceKHR GetSurface() const;

    FVulkanDevice* GetLogicalDevice() const;

  protected:
    VkPhysicalDevice device;
    VkSurfaceKHR surface;

    std::vector<const char*> layers;

    std::unique_ptr<FVulkanDevice> logicalDevice;

  private:
};
