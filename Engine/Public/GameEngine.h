#include "GLFW/glfw3.h"
#include <array>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Vulkan/QueueFamilyIndices.h"
#include "Vulkan/SwapChainSupportDetails.h"
#include "VulkanRHI/VulkanRHI.h"

class GameEngine
{
  public:
    GameEngine();
    ~GameEngine();

  public:
    void run();

  private:
    void init();
    void initWindow();
    void initRHI();

    void createWindowSurface();

    void tick();
    void cleanup();

  private:
    std::shared_ptr<FVulkanRHI> RHI;

  private:
    // Vulkan related initialization
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device);
    bool checkValidationLayerSupport() const;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;

  private:
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;

    VkSurfaceKHR surface;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
};
