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

  private:
    void CreateWindow();
};
