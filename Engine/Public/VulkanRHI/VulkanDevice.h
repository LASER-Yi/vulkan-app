#pragma once

#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/SwapChainSupportDetails.h"
#include "vulkan/vulkan_core.h"

#include <memory>
#include <string>
#include <vector>

class FVulkanGpu;
class FVulkanShader;
class FVulkanSwapChain;

class FVulkanDevice
{
  public:
    FVulkanDevice(VkDevice device, FVulkanGpu* physicalDevice);
    ~FVulkanDevice();

    std::shared_ptr<FVulkanShader> CreateShader(const std::string& filename,
                                                VkShaderStageFlagBits stage);

    VkDevice GetDevice() const { return device; }

    FVulkanSwapChain* GetSwapChain() const { return swapChain.get(); }

    VkRenderPass GetRenderPass() const { return renderPass; }

  protected:
    FVulkanGpu* physicalDevice;

    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    std::unique_ptr<FVulkanSwapChain> swapChain;

    std::vector<std::shared_ptr<FVulkanShader>> shaders;

    // TODO: Move to separate class
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkRenderPass renderPass;

  private:
    void InitSwapChain();
    void InitDeviceQueue();
    void InitPipeline();
    void InitRenderPass();
};
