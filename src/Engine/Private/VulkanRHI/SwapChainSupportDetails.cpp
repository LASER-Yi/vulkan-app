#include "VulkanRHI/SwapChainSupportDetails.h"

#include "GLFW/glfw3.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <stdint.h>
#include <vulkan/vulkan.hpp>

#include "VulkanRHI/VulkanCommon.h"

FSwapChainSupportDetails::FSwapChainSupportDetails(vk::PhysicalDevice device,
                                                   vk::SurfaceKHR surface)
    : device(device), surface(surface)
{
    VERIFY_VULKAN_RESULT(
        device.getSurfaceCapabilitiesKHR(surface, &capabilities));

    // Get the number of formats supported by the surface
    formats = device.getSurfaceFormatsKHR(surface);

    // Get the number of present modes supported by the surface
    presentModes = device.getSurfacePresentModesKHR(surface);
}

vk::SurfaceFormatKHR FSwapChainSupportDetails::GetRequiredSurfaceFormat() const
{
    for (const vk::SurfaceFormatKHR& format : formats) {
        if (format.format == vk::Format::eR8G8B8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return formats[0];
}

vk::PresentModeKHR FSwapChainSupportDetails::GetRequiredPresentMode() const
{
    /*
        VK_PRESENT_MODE_IMMEDIATE_KHR: No buffer
        VK_PRESENT_MODE_FIFO_KHR: Double buffer
        VK_PRESENT_MODE_MAILBOX_KHR: Triple buffer
    */

    bool HasTripleBuffer = false;
    bool HasDoubleBuffer = false;

    for (const vk::PresentModeKHR& mode : presentModes) {
        switch (mode) {
        case vk::PresentModeKHR::eMailbox:
            HasTripleBuffer = true;
            break;
        case vk::PresentModeKHR::eFifo:
            HasDoubleBuffer = true;
            break;
        default:
            break;
        }
    }

    if (HasDoubleBuffer) {
        return vk::PresentModeKHR::eFifo;
    } else if (HasTripleBuffer) {
        return vk::PresentModeKHR::eMailbox;
    }

    return presentModes[0];
}

vk::Extent2D
FSwapChainSupportDetails::GetRequiredExtent(GLFWwindow* window) const
{
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        assert(false);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        vk::Extent2D actualExtent = {.width = static_cast<uint32_t>(width),
                                     .height = static_cast<uint32_t>(height)};

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

uint32_t FSwapChainSupportDetails::GetImageCount() const
{
    const uint32_t min = capabilities.minImageCount;
    const uint32_t max = capabilities.maxImageCount;
    return std::clamp(min + 1, min, max);
}

bool FSwapChainSupportDetails::IsValid() const
{
    return !formats.empty() && !presentModes.empty();
}
