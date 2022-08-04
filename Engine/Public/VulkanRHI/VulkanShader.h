#pragma once

#include "Core/FileManager.h"
#include <memory>
#include <string>
#include <vulkan/vulkan_core.h>

class FVulkanDevice;

class FVulkanShader
{
  public:
    FVulkanShader(FVulkanDevice* device, VkShaderModule shader,
                  VkShaderStageFlagBits stage, const std::string& entryPoint);
    ~FVulkanShader();

    void CreatePipelineStage() const;

  private:
    FVulkanDevice* device;
    VkShaderModule ShaderModule;

    VkShaderStageFlagBits stage;
    std::string entryPoint;
};
