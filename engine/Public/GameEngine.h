#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

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

    void createWindowSurface();

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

    VkSurfaceKHR surface;
};
