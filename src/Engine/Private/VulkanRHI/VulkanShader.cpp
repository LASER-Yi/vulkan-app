#include "VulkanRHI/VulkanShader.h"

#include "Core/FileManager.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

FVulkanShader::FVulkanShader(FVulkanDevice* device, const FileBlob& blob,
                             vk::ShaderStageFlagBits stage,
                             const std::string& entryPoint)
    : device(device), stage(stage), entryPoint(entryPoint)
{
    const vk::ShaderModuleCreateInfo createInfo = {
        .sType = vk::StructureType::eShaderModuleCreateInfo,
        .codeSize = blob.GetFileSize(),
        .pCode = blob.GetData(),
    };

    auto vk_device = device->GetDevice();

    VERIFY_VULKAN_RESULT(
        vk_device.createShaderModule(&createInfo, nullptr, &ShaderModule));
}

FVulkanShader::~FVulkanShader()
{
    vk::Device vk_device = device->GetDevice();
    vk_device.destroyShaderModule(ShaderModule);

    device = VK_NULL_HANDLE;
}

vk::PipelineShaderStageCreateInfo FVulkanShader::CreatePipelineStage() const
{
    const vk::PipelineShaderStageCreateInfo stageInfo = {
        .sType = vk::StructureType::ePipelineShaderStageCreateInfo,
        .stage = stage,
        .module = ShaderModule,
        .pName = entryPoint.c_str(),
    };

    return stageInfo;
}
