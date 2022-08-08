#pragma once

#include "GLFW/glfw3.h"
#include <stdint.h>
#include <vector>
#include <vulkan/vulkan.hpp>

class GLFWwindow;

struct FSwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;

    vk::PhysicalDevice device;
    vk::SurfaceKHR surface;

  public:
    FSwapChainSupportDetails(const vk::PhysicalDevice device,
                             const vk::SurfaceKHR surface);

    vk::SurfaceKHR GetSurface() const { return surface; }

    vk::SurfaceFormatKHR GetRequiredSurfaceFormat() const;
    vk::PresentModeKHR GetRequiredPresentMode() const;
    vk::Extent2D GetRequiredExtent(GLFWwindow* window) const;
    uint32_t GetImageCount() const;
    bool IsValid() const;
};
