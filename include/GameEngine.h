#define VK_USE_PLATFORM_MACOS_KHR
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_MACOS
#include "GLFW/glfw3native.h"

#include "Vulkan/QueueFamilyIndices.h"

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

    void render();
    void cleanup();

  private:
    // Vulkan related initialization
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device);
    bool checkValidationLayerSupport();

  private:
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
};
