#include "Vulkan/QueueFamilyIndices.h"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class FVulkanInstance;

// Physical Device Wrapper
class FVulkanGPU
{
  public:
    FVulkanGPU(VkPhysicalDevice device);
    ~FVulkanGPU();

    void SetOwner(FVulkanInstance* Instance);

    QueueFamilyIndices GetQueueFamilies() const;
    const VkPhysicalDeviceProperties GetProperties() const;
    const VkPhysicalDeviceFeatures GetFeatures() const;

    std::vector<VkExtensionProperties> GetExtensions() const;

    bool IsValid() const;
    uint32_t GetScore() const;

  protected:
    VkPhysicalDevice device;

    FVulkanInstance* Owner;
};
