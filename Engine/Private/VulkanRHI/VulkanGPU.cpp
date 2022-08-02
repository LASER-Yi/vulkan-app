#include "VulkanRHI/VulkanGPU.h"

#include <assert.h>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanRHI/VulkanInstance.h"

const std::vector<const char*> requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

FVulkanGPU::FVulkanGPU(VkPhysicalDevice device) : device(device) {}

FVulkanGPU::~FVulkanGPU() { device = VK_NULL_HANDLE; }

void FVulkanGPU::SetOwner(FVulkanInstance* Instance) { Owner = Instance; }

QueueFamilyIndices FVulkanGPU::GetQueueFamilies() const
{

    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const VkQueueFamilyProperties& properties : queueFamilies) {
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;

        // TODO
        assert(Owner != nullptr);

        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, Owner->GetSurface(),
                                             &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        i += 1;
    }

    return indices;
}

const VkPhysicalDeviceProperties FVulkanGPU::GetProperties() const
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    return std::move(properties);
}

const VkPhysicalDeviceFeatures FVulkanGPU::GetFeatures() const
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    return std::move(features);
}

std::vector<VkExtensionProperties> FVulkanGPU::GetExtensions() const
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         extensions.data());

    return std::move(extensions);
}

bool FVulkanGPU::IsValid() const
{
    const auto extensions = GetExtensions();

    std::set<std::string> requiredExts(requiredExtensions.begin(),
                                       requiredExtensions.end());

    for (const VkExtensionProperties& extensionProperties : extensions) {
        requiredExts.erase(extensionProperties.extensionName);
    }

    const bool IsExtensionAvailable = requiredExtensions.empty();

    const QueueFamilyIndices indices = GetQueueFamilies();

    return IsExtensionAvailable && indices.isValid();
}

uint32_t FVulkanGPU::GetScore() const
{
    const VkPhysicalDeviceProperties properties = GetProperties();
    const VkPhysicalDeviceFeatures features = GetFeatures();

    uint32_t score = 0;

    switch (properties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        score += 1000;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        score += 100;
        break;
    default:
        break;
    }

    return score;
}
