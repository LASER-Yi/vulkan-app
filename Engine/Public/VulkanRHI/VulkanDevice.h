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

    VkQueue GetGraphicsQueue() const { return graphicsQueue; }
    VkQueue GetPresentQueue() const { return presentQueue; }

    VkDevice GetDevice() const { return device; }

    FVulkanSwapChain* GetSwapChain() const { return swapChain.get(); }

    void WaitRenderFinished();

    VkRenderPass GetRenderPass() const { return renderPass; }

    VkCommandBuffer CreateCommandBuffer();

    void Submit(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    VkFence TEMP_GetRenderFence() const { return inRenderFence; }

  protected:
    FVulkanGpu* physicalDevice;

    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    std::unique_ptr<FVulkanSwapChain> swapChain;

    std::vector<std::shared_ptr<FVulkanShader>> shaders;

    // TODO: Move to separate class
    VkViewport viewport;
    VkRect2D scissor;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkRenderPass renderPass;

    VkCommandPool commandPool;

    VkFence inRenderFence;

  private:
    void InitSwapChain();
    void InitDeviceQueue();
    void InitPipeline();
    void InitRenderPass();

    void InitCommandPool();

    void InitFences();
};
