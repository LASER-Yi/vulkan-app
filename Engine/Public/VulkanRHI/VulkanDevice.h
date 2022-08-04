#pragma once

#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/SwapChainSupportDetails.h"
#include "vulkan/vulkan_core.h"

#include <memory>
#include <string>
#include <vector>

class FVulkanShader;
class FVulkanSwapChain;

class FVulkanDevice
{
  public:
    FVulkanDevice(VkDevice device, const QueueFamilyIndices& indices);
    ~FVulkanDevice();

    std::shared_ptr<FVulkanShader> CreateShader(const std::string& filename,
                                                VkShaderStageFlagBits stage);

    void InitSwapChain(const SwapChainSupportDetails& details);

    VkDevice GetDevice() const { return device; }

    FVulkanSwapChain* GetSwapChain() const { return swapChain.get(); }

    VkRenderPass GetRenderPass() const { return renderPass; }

  protected:
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    std::unique_ptr<FVulkanSwapChain> swapChain;

    QueueFamilyIndices indices;
    std::vector<std::shared_ptr<FVulkanShader>> shaders;

    // TODO: Move to separate class
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkRenderPass renderPass;

  private:
    void InitDeviceQueue();
    void InitPipeline();
    void InitRenderPass();
};
