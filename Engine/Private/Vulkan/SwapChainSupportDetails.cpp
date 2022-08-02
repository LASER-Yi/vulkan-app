#include "Vulkan/SwapChainSupportDetails.h"
#include "GLFW/glfw3.h"
#include <algorithm>
#include <limits>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

SwapChainSupportDetails::SwapChainSupportDetails(const VkPhysicalDevice device,
                                                 const VkSurfaceKHR surface)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    // Get the number of formats supported by the surface
    {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                             nullptr);
        if (formatCount != 0) {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                                 formats.data());
        }
    }

    // Get the number of present modes supported by the surface
    {
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                                  &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device, surface, &presentModeCount, presentModes.data());
        }
    }
}

VkSurfaceFormatKHR SwapChainSupportDetails::GetRequiredSurfaceFormat() const
{
    for (const VkSurfaceFormatKHR& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return formats[0];
}

VkPresentModeKHR SwapChainSupportDetails::GetRequiredPresentMode() const
{
    /*
        VK_PRESENT_MODE_IMMEDIATE_KHR: No buffer
        VK_PRESENT_MODE_FIFO_KHR: Double buffer
        VK_PRESENT_MODE_MAILBOX_KHR: Triple buffer
    */

    bool HasTripleBuffer = false;
    bool HasDoubleBuffer = false;

    for (const VkPresentModeKHR& mode : presentModes) {
        switch (mode) {
        case VK_PRESENT_MODE_MAILBOX_KHR:
            HasTripleBuffer = true;
            break;
        case VK_PRESENT_MODE_FIFO_KHR:
            HasDoubleBuffer = true;
            break;
        default:
            break;
        }
    }

    if (HasTripleBuffer) {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    } else if (HasDoubleBuffer) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    return presentModes[0];
}

VkExtent2D SwapChainSupportDetails::GetRequiredExtent(GLFWwindow* window) const
{
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                                   static_cast<uint32_t>(height)};

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

uint32_t SwapChainSupportDetails::GetImageCount() const
{
    const uint32_t min = capabilities.minImageCount;
    const uint32_t max = capabilities.maxImageCount;
    return std::clamp(min + 1, min, max);
}

bool SwapChainSupportDetails::IsValid() const
{
    return !formats.empty() && !presentModes.empty();
}
