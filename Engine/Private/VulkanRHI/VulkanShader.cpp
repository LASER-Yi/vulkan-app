#include "VulkanRHI/VulkanShader.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

FVulkanShader::FVulkanShader(FVulkanDevice* device, VkShaderModule shader)
    : device(device), ShaderModule(shader)
{
}

FVulkanShader::~FVulkanShader()
{
    VkDevice _device = device->GetDevice();
    vkDestroyShaderModule(_device, ShaderModule, nullptr);
    ShaderModule = VK_NULL_HANDLE;

    device = VK_NULL_HANDLE;
}
