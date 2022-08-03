#include "Core/FileManager.h"
#include <memory>
#include <string>
#include <vulkan/vulkan_core.h>

class FVulkanShader
{
  public:
    FVulkanShader(VkShaderModule shader);
    ~FVulkanShader();

    static std::shared_ptr<FVulkanShader> Create(VkDevice device,
                                                 const std::string& filename);

  private:
    VkDevice logicalDevice;
    VkShaderModule ShaderModule;
};
