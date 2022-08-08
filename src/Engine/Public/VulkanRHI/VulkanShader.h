#pragma once

#include "Core/FileManager.h"
#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>

class FVulkanDevice;

class FVulkanShader
{
  public:
    FVulkanShader(FVulkanDevice* device, const FileBlob& blob,
                  vk::ShaderStageFlagBits stage, const std::string& entryPoint);
    ~FVulkanShader();

    vk::PipelineShaderStageCreateInfo CreatePipelineStage() const;

  private:
    FVulkanDevice* device;
    vk::ShaderModule ShaderModule;

    vk::ShaderStageFlagBits stage;
    std::string entryPoint;
};
