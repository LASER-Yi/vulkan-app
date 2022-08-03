#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class FVulkanGPU;
class FVulkanGPUCreateParam;
class FVulkanRHI;
class GLFWwindow;

class FVulkanInstance
{
  public:
    FVulkanInstance();
    ~FVulkanInstance();

    static std::vector<const char*> validationLayers;

    void Init(GLFWwindow* window);

  public:
    std::vector<FVulkanGPUCreateParam> GetGPUs() const;

  protected:
    VkInstance instance;
    VkSurfaceKHR surface;
    std::shared_ptr<FVulkanGPU> device;

    std::vector<const char*> enabledLayers;
    std::vector<const char*> enabledExtensions;

  private:
    bool SupportValidationLayer() const;

    void SelectGPU();
    void CreateInstance();
    void CreateSurface(GLFWwindow* window);
};
