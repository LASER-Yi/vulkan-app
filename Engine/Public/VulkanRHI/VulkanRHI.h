
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

class FVulkanInstance;
class GLFWwindow;

class FVulkanRHI
{
  public:
    FVulkanRHI();
    ~FVulkanRHI();

    void Init();
    void Render();

    std::shared_ptr<FVulkanInstance> GetInstance() const;

    static std::vector<VkExtensionProperties> GetAvailableExtensions();
    static std::vector<VkLayerProperties> GetAvailableLayers();

  protected:
    std::shared_ptr<FVulkanInstance> Instance;
    GLFWwindow* window;

  private:
    void CreateWindow();
};
