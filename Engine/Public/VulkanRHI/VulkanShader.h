#pragma once

#include "Core/FileManager.h"
#include <memory>
#include <string>
#include <vulkan/vulkan_core.h>

class FVulkanDevice;

class FVulkanShader
{
  public:
    FVulkanShader(FVulkanDevice* device, VkShaderModule shader);
    ~FVulkanShader();

  private:
    FVulkanDevice* device;
    VkShaderModule ShaderModule;
};
