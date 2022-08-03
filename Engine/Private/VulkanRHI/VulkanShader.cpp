#include "VulkanRHI/VulkanShader.h"
#include "VulkanRHI/VulkanCommon.h"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

FVulkanShader::FVulkanShader(VkShaderModule shader) : ShaderModule(shader) {}

FVulkanShader::~FVulkanShader()
{
    vkDestroyShaderModule(logicalDevice, ShaderModule, nullptr);
    ShaderModule = VK_NULL_HANDLE;

    logicalDevice = VK_NULL_HANDLE;
}

std::shared_ptr<FVulkanShader>
FVulkanShader::Create(VkDevice device, const std::string& filename)
{
    const FileBlob blob = FileManager::ReadFile(filename);

    VkShaderModuleCreateInfo createInfo = {};
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = blob.GetFileSize();
    createInfo.pCode = blob.GetData();

    VkShaderModule shader;
    const VkResult CreateResult =
        vkCreateShaderModule(device, &createInfo, nullptr, &shader);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }

    auto pShader = std::make_shared<FVulkanShader>(shader);
    pShader->logicalDevice = device;

    return pShader;
}
