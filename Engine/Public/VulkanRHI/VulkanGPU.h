#include <vector>
#include <vulkan/vulkan_core.h>

#include "Vulkan/QueueFamilyIndices.h"

class FVulkanInstance;

struct FVulkanGPUCreateParam {

public:
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
};

// Physical Device Wrapper
class FVulkanGPU
{
  public:
    FVulkanGPU(VkPhysicalDevice device, VkSurfaceKHR surface);
    ~FVulkanGPU();

    void Init();
    bool IsInit() const;

    QueueFamilyIndices GetQueueFamilies() const;
    const VkPhysicalDeviceProperties GetProperties() const;
    const VkPhysicalDeviceFeatures GetFeatures() const;

    std::vector<VkExtensionProperties> GetExtensions() const;

    bool IsValid() const;
    uint32_t GetScore() const;

  protected:
    VkPhysicalDevice device;
    VkSurfaceKHR surface;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkDevice logicalDevice;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    void CreateSwapChain();
};
