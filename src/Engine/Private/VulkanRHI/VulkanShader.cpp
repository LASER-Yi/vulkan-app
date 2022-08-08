#include "VulkanRHI/VulkanShader.h"

#include "Core/FileManager.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

FVulkanShader::FVulkanShader(FVulkanDevice* device, const FileBlob& blob,
                             VkShaderStageFlagBits stage,
                             const std::string& entryPoint)
    : device(device), stage(stage), entryPoint(entryPoint)
{
    VkShaderModuleCreateInfo createInfo = {};
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = blob.GetFileSize();
    createInfo.pCode = blob.GetData();

    VkShaderModule shader;
    const VkResult CreateResult = vkCreateShaderModule(
        device->GetDevice(), &createInfo, nullptr, &ShaderModule);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }
}

FVulkanShader::~FVulkanShader()
{
    VkDevice _device = device->GetDevice();
    vkDestroyShaderModule(_device, ShaderModule, nullptr);
    ShaderModule = VK_NULL_HANDLE;

    device = VK_NULL_HANDLE;
}

VkPipelineShaderStageCreateInfo FVulkanShader::CreatePipelineStage() const
{
    VkDevice _device = device->GetDevice();

    VkPipelineShaderStageCreateInfo stageInfo = {};
    ZeroVulkanStruct(stageInfo,
                     VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = stage;
    stageInfo.module = ShaderModule;
    stageInfo.pName = entryPoint.c_str();

    return std::move(stageInfo);
}
