#pragma once

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/SwapChainSupportDetails.h"
#include "VulkanRHI/VulkanDevice.h"
#include "VulkanRHI/VulkanShader.h"

class FVulkanInstance;

// Physical Device Wrapper
class FVulkanGpu
{
  public:
    FVulkanGpu(vk::PhysicalDevice device, vk::SurfaceKHR* surface,
               const std::vector<const char*>& layers);
    ~FVulkanGpu();

    void InitLogicalDevice();

    FQueueFamilyIndices GetQueueFamilies() const;
    const vk::PhysicalDeviceProperties GetProperties() const;
    const vk::PhysicalDeviceFeatures GetFeatures() const;

    std::vector<vk::ExtensionProperties> GetExtensions() const;

    FSwapChainSupportDetails GetSwapChainSupportDetails() const;

    bool IsValid() const;
    uint32_t GetScore() const;

    vk::SurfaceKHR* GetSurface() const;

    FVulkanDevice* GetLogicalDevice() const;

  protected:
    vk::PhysicalDevice device;
    vk::SurfaceKHR* surface;

    std::vector<const char*> layers;

    std::unique_ptr<FVulkanDevice> logicalDevice;

  private:
};
