#include "Definition.h"
#include "GameEngine.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <vector>
#include <vulkan/vulkan_core.h>

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

void GameEngine::createInstance()
{
    if (GE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
        throw std::runtime_error(
            "Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Game Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Game Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           extensions.data());

    std::cout << "Available extensions:" << std::endl;
    for (const VkExtensionProperties& extension : extensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
    }

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensionNames = {};
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions =
            glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (size_t i = 0; i < glfwExtensionCount; i++) {
            extensionNames.push_back(glfwExtensions[i]);
        }
    }

    std::vector<const char*> layerNames = {};

    if (GE_VALIDATION_LAYERS) {
        layerNames.insert(layerNames.end(), validationLayers.begin(),
                          validationLayers.end());
    }

// Workaround for VK_ERROR_INCOMPATIBLE_DRIVER
#if WITH_MOLTEN_VK
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    extensionNames.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensionNames.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    createInfo.enabledLayerCount = layerNames.size();
    createInfo.ppEnabledLayerNames = layerNames.data();

    const VkResult CreateResult =
        vkCreateInstance(&createInfo, nullptr, &instance);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

void GameEngine::pickPhysicalDevice()
{
    assert(instance != VK_NULL_HANDLE);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    const auto suitable = [this](const VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // only check for discrete graphic cards is a bad idea
        // TODO: Don't know why M1's GPU is not support geometry shader, check
        // later;

        // return deviceFeatures.geometryShader;

        QueueFamilyIndices indices = findQueueFamilies(device);

        const bool isExtensionSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (isExtensionSupported) {
            SwapChainSupportDetails swapChainSupport(device, surface);

            swapChainAdequate = !swapChainSupport.formats.empty() &&
                                !swapChainSupport.presentModes.empty();
        }

        return indices.isValid() && isExtensionSupported && swapChainAdequate;
    };

    for (const VkPhysicalDevice& device : devices) {
        if (suitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

void GameEngine::createLogicalDevice()
{
    assert(physicalDevice != VK_NULL_HANDLE);

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value()};

    const float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    if (GE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    std::vector<const char*> extensionNames = {};

    // Swapchain
    {
        extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = extensionNames.size();
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    const VkResult CreateResult =
        vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0,
                     &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0,
                     &presentQueue);
}

void GameEngine::createSwapChain()
{
    const SwapChainSupportDetails details(physicalDevice, surface);

    assert(details.IsValid());

    const VkSurfaceFormatKHR surfaceFormat = details.GetRequiredSurfaceFormat();

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = details.GetImageCount();

    createInfo.presentMode = details.GetRequiredPresentMode();
    createInfo.clipped = VK_TRUE;

    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;

    createInfo.imageExtent = details.GetRequiredExtent(window);

    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                           indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    const VkResult CreateResult =
        vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }
}

QueueFamilyIndices GameEngine::findQueueFamilies(const VkPhysicalDevice device)
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        i += 1;
    }

    return indices;
}

bool GameEngine::checkValidationLayerSupport() const
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

#if BUILD_DEBUG
    std::cout << "Available layers:" << std::endl;
    for (const VkLayerProperties& layerProperties : availableLayers) {

        std::cout << '\t' << layerProperties.layerName << std::endl;
    }
#endif

    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const VkLayerProperties& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

bool GameEngine::checkDeviceExtensionSupport(VkPhysicalDevice device) const
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const VkExtensionProperties& extensionProperties :
         availableExtensions) {
        requiredExtensions.erase(extensionProperties.extensionName);
    }

    return requiredExtensions.empty();
}
