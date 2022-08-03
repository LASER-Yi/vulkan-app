#pragma once

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/VulkanShader.h"

class FVulkanInstance;
class FVulkanSwapChain;

struct FVulkanGPUCreateParam {
    VkPhysicalDevice device;
    VkSurfaceKHR surface;

    std::vector<const char*> layers;

    FVulkanGPUCreateParam();
    QueueFamilyIndices GetQueueFamilies() const;
    const VkPhysicalDeviceProperties GetProperties() const;
    const VkPhysicalDeviceFeatures GetFeatures() const;

    std::vector<VkExtensionProperties> GetExtensions() const;

    bool IsValid() const;
    uint32_t GetScore() const;
};

// Physical Device Wrapper
class FVulkanGPU
{
  public:
    FVulkanGPU(const FVulkanGPUCreateParam& Param);
    ~FVulkanGPU();

  protected:
    VkPhysicalDevice device;
    QueueFamilyIndices indices;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkDevice logicalDevice;

    std::shared_ptr<FVulkanSwapChain> swapChain;

  private:
    void Init(const FVulkanGPUCreateParam& Param);
    void CreateLogicalDevice(const FVulkanGPUCreateParam& Param);
    void CreateSwapChain(const FVulkanGPUCreateParam& Param);
    void CreateDeviceQueue();
};
