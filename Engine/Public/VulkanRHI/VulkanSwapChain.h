#include <vulkan/vulkan.h>

#include <vector>

class FVulkanSwapChain
{
  public:
    FVulkanSwapChain(VkSwapchainKHR swapChain);
    ~FVulkanSwapChain();

    void Init(VkDevice logicalDevice,
              const VkSwapchainCreateInfoKHR& CreateInfo);
    void Deinit(VkDevice logicalDevice);

  protected:
    VkSwapchainKHR swapChain;
    std::vector<VkImage> Images;
    VkFormat ImageFormat;
    VkExtent2D Extent;
};
