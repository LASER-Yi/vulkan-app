#include "VulkanRHI/VulkanShader.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

FVulkanShader::FVulkanShader(FVulkanDevice* device, VkShaderModule shader,
                             VkShaderStageFlagBits stage,
                             const std::string& entryPoint)
    : device(device), ShaderModule(shader), stage(stage), entryPoint(entryPoint)
{
}

FVulkanShader::~FVulkanShader()
{
    VkDevice _device = device->GetDevice();
    vkDestroyShaderModule(_device, ShaderModule, nullptr);
    ShaderModule = VK_NULL_HANDLE;

    device = VK_NULL_HANDLE;
}

void FVulkanShader::CreatePipelineStage() const
{
    VkDevice _device = device->GetDevice();

    VkPipelineShaderStageCreateInfo stageInfo = {};
    ZeroVulkanStruct(stageInfo,
                     VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = stage;
    stageInfo.module = ShaderModule;
    stageInfo.pName = entryPoint.c_str();
}
