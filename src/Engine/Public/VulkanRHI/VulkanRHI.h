#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "GLFW/glfw3.h"
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

    static std::vector<vk::ExtensionProperties> GetAvailableExtensions();
    static std::vector<vk::LayerProperties> GetAvailableLayers();

  private:
    std::unique_ptr<FVulkanInstance> Instance;
    GLFWwindow* window;

    // TODO: refactor this
    vk::CommandBuffer commandBuffer;

  private:
    void CreateWindow();

    void Draw();

    static void OnFramebufferResize(GLFWwindow* window, int width, int height);
};
