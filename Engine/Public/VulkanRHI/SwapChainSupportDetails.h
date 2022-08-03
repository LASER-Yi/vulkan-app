#pragma once

#include "GLFW/glfw3.h"
#include <stdint.h>
#include <vector>
#include <vulkan/vulkan_core.h>

class GLFWwindow;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    VkPhysicalDevice device;
    VkSurfaceKHR surface;

  public:
    SwapChainSupportDetails(const VkPhysicalDevice device,
                            const VkSurfaceKHR surface);

    VkSurfaceKHR GetSurface() const { return surface; }

    VkSurfaceFormatKHR GetRequiredSurfaceFormat() const;
    VkPresentModeKHR GetRequiredPresentMode() const;
    VkExtent2D GetRequiredExtent(GLFWwindow* window) const;
    uint32_t GetImageCount() const;
    bool IsValid() const;
};
