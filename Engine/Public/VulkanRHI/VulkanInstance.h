#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class FVulkanGPU;
class FVulkanRHI;
class GLFWwindow;

class FVulkanInstance
{
  public:
    FVulkanInstance();
    ~FVulkanInstance();

    void SetOwner(FVulkanRHI* RHI);

    static std::vector<const char*> validationLayers;

    void Init(GLFWwindow* window);

  public:
    VkInstance Get() const;
    std::vector<FVulkanGPU> GetGPUs() const;

    VkSurfaceKHR GetSurface() const;

  protected:
    VkInstance instance;
    VkSurfaceKHR surface;

    std::shared_ptr<FVulkanGPU> device;

    FVulkanRHI* Owner;

  private:
    bool SupportValidationLayer() const;

    void SelectGPU();
    void CreateInstance();
    void CreateSurface(GLFWwindow* window);
};
