#pragma once

#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/SwapChainSupportDetails.h"
#include <vulkan/vulkan.hpp>

#include <memory>
#include <string>
#include <vector>

class FVulkanGpu;
class FVulkanShader;
class FVulkanSwapChain;

class FVulkanDevice
{
  public:
    FVulkanDevice(vk::Device device, FVulkanGpu* physicalDevice);
    ~FVulkanDevice();

    std::shared_ptr<FVulkanShader> CreateShader(const std::string& filename,
                                                vk::ShaderStageFlagBits stage);

    vk::Queue* GetGraphicsQueue() { return &graphicsQueue; }
    vk::Queue* GetPresentQueue() { return &presentQueue; }

    vk::Device& GetDevice() { return device; }

    FVulkanGpu* GetPhysicalDevice() const { return physicalDevice; }

    FVulkanSwapChain* GetSwapChain() const { return swapChain.get(); }

    vk::RenderPass GetRenderPass() const { return renderPass; }

    void BeginNextFrame();

    vk::CommandBuffer CreateCommandBuffer();

    void Render(vk::CommandBuffer* commandBuffer);
    void Submit(vk::CommandBuffer* commandBuffer);

  protected:
    FVulkanGpu* physicalDevice;

    vk::Device device;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    std::unique_ptr<FVulkanSwapChain> swapChain;

    std::vector<std::shared_ptr<FVulkanShader>> shaders;

    // TODO: Move to separate class
    vk::Viewport viewport;
    vk::Rect2D scissor;

    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;

    vk::RenderPass renderPass;

    vk::CommandPool commandPool;

    vk::Fence inRenderFence;

  private:
    FSwapChainSupportDetails swapChainDetails;

  private:
    void InitSwapChain();
    void InitDeviceQueue();
    void InitPipeline();
    void InitRenderPass();

    void InitCommandPool();

    void InitFences();
};
