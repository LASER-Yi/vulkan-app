#include "VulkanRHI/VulkanInstance.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "Definition.h"
#include "GLFW/glfw3.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanGPU.h"
#include "VulkanRHI/VulkanRHI.h"

std::vector<const char*> FVulkanInstance::validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

FVulkanInstance::FVulkanInstance(GLFWwindow* window)
    : instance(VK_NULL_HANDLE), surface(VK_NULL_HANDLE)
{
    assert(window != nullptr);

    CreateInstance();
    CreateSurface(window);
    SelectGPU();
}

FVulkanInstance::~FVulkanInstance()
{
    device = nullptr;

    instance.destroy();
}

std::vector<std::unique_ptr<FVulkanGpu>> FVulkanInstance::GetGPUs()
{
    std::vector<vk::PhysicalDevice> devices =
        instance.enumeratePhysicalDevices();

    if (devices.size() == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<std::unique_ptr<FVulkanGpu>> gpus;
    gpus.reserve(devices.size());
    for (auto& device : devices) {
        gpus.emplace_back(
            std::make_unique<FVulkanGpu>(device, this, enabledLayers));
    }

    return std::move(gpus);
}

bool FVulkanInstance::SupportValidationLayer() const
{
    const std::vector<vk::LayerProperties> availableLayers =
        FVulkanRHI::GetAvailableLayers();

#if BUILD_DEBUG
    std::cout << "Available layers:" << std::endl;
    for (const vk::LayerProperties& layerProperties : availableLayers) {

        std::cout << '\t' << layerProperties.layerName << std::endl;
    }
#endif

    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const vk::LayerProperties& layerProperties : availableLayers) {
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

void FVulkanInstance::SelectGPU()
{
    auto gpus = GetGPUs();

    // Simplest solution
    for (std::unique_ptr<FVulkanGpu>& gpu : gpus) {
        if (gpu->IsValid()) {
            device = std::move(gpu);
            break;
        }
    }

    if (device == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    device->InitLogicalDevice();
}

void FVulkanInstance::CreateInstance()
{
    if (GE_VALIDATION_LAYERS && !SupportValidationLayer()) {
        throw std::runtime_error(
            "Validation layers requested, but not available!");
    }

    static const std::string appName = "Vulkan App";
    static const std::string engineName = "Vulkan Engine";

    vk::ApplicationInfo appInfo = {.sType = vk::StructureType::eApplicationInfo,
                                   .pApplicationName = appName.c_str(),
                                   .applicationVersion =
                                       VK_MAKE_VERSION(1, 0, 0),
                                   .pEngineName = engineName.c_str(),
                                   .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                   .apiVersion = VK_API_VERSION_1_2};

    std::vector<vk::ExtensionProperties> extensions =
        FVulkanRHI::GetAvailableExtensions();

#if BUILD_DEBUG
    std::cout << "Available extensions:" << std::endl;
    for (const vk::ExtensionProperties& extension : extensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
    }
#endif

    vk::InstanceCreateInfo createInfo = {
        .sType = vk::StructureType::eInstanceCreateInfo,
        .pApplicationInfo = &appInfo};

    enabledExtensions.clear();
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions =
            glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (size_t i = 0; i < glfwExtensionCount; i++) {
            enabledExtensions.push_back(glfwExtensions[i]);
        }
    }

    enabledLayers.clear();

    if (GE_VALIDATION_LAYERS) {
        enabledLayers.insert(enabledLayers.end(), validationLayers.begin(),
                             validationLayers.end());
    }

// Workaround for VK_ERROR_INCOMPATIBLE_DRIVER
#if WITH_MOLTEN_VK
    createInfo.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    enabledExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    enabledExtensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = enabledExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    createInfo.enabledLayerCount = enabledLayers.size();
    createInfo.ppEnabledLayerNames = enabledLayers.data();

    VERIFY_VULKAN_RESULT(vk::createInstance(&createInfo, nullptr, &instance));
}

void FVulkanInstance::CreateSurface(GLFWwindow* window)
{
    VkSurfaceKHR c_surface;
    const VkResult Result =
        glfwCreateWindowSurface(instance, window, nullptr, &c_surface);

    surface = c_surface;
}
