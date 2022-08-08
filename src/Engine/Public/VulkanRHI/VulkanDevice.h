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

    FVulkanGpu* GetPhysicalDevice() const { return physicalDevice; }

    FVulkanSwapChain* GetSwapChain() const { return swapChain.get(); }

    VkRenderPass GetRenderPass() const { return renderPass; }

    void BeginNextFrame();

    VkCommandBuffer CreateCommandBuffer();

    void Render(VkCommandBuffer commandBuffer);
    void Submit(VkCommandBuffer commandBuffer);

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