#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "GLFW/glfw3.h"
#include "VulkanRHI/VulkanGPU.h"

class FVulkanRHI;

class FVulkanInstance
{
  public:
    FVulkanInstance(GLFWwindow* window);
    ~FVulkanInstance();

    static std::vector<const char*> validationLayers;

  public:
    std::vector<std::unique_ptr<FVulkanGpu>> GetGPUs();

    FVulkanGpu* GetPhysicalDevice() const { return device.get(); }

    vk::SurfaceKHR GetSurface() const { return surface; }

  protected:
    vk::Instance instance;
    vk::SurfaceKHR surface;
    std::unique_ptr<FVulkanGpu> device;

    std::vector<const char*> enabledLayers;
    std::vector<const char*> enabledExtensions;

  private:
    bool SupportValidationLayer() const;

    void SelectGPU();
    void CreateInstance();
    void CreateSurface(GLFWwindow* window);
};
