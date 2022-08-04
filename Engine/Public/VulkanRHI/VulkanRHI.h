#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanInstance.h"

class GLFWwindow;

class FVulkanRHI
{
  public:
    FVulkanRHI();

    void Init();
    void Destroy();

    void Render();

    FVulkanInstance* GetInstance() const;

    static std::vector<VkExtensionProperties> GetAvailableExtensions();
    static std::vector<VkLayerProperties> GetAvailableLayers();

  protected:
    std::unique_ptr<FVulkanInstance> Instance;
    GLFWwindow* window;

    // TODO: Move to separate class
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkRenderPass renderPass;

  private:
    void CreateWindow();
    void CreatePipeline();
    void CreateRenderPass();
};
