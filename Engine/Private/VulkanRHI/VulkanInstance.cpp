#include "VulkanRHI/VulkanInstance.h"

#include <iostream>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "Definition.h"
#include "GLFW/glfw3.h"
#include "VulkanRHI/VulkanGPU.h"
#include "VulkanRHI/VulkanRHI.h"

std::vector<const char*> FVulkanInstance::validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

FVulkanInstance::FVulkanInstance() {}

FVulkanInstance::~FVulkanInstance()
{
    device.reset();
    vkDestroyInstance(instance, nullptr);
}

std::vector<FVulkanGPUCreateParam> FVulkanInstance::GetGPUs() const
{
    assert(instance != VK_NULL_HANDLE);
    assert(surface != VK_NULL_HANDLE);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::vector<FVulkanGPUCreateParam> gpus;
    gpus.reserve(devices.size());
    for (const auto& device : devices) {
        FVulkanGPUCreateParam param;
        param.device = device;
        param.surface = surface;
        param.layers = enabledLayers;

        gpus.emplace_back(param);
    }

    return std::move(gpus);
}

bool FVulkanInstance::SupportValidationLayer() const
{
    const std::vector<VkLayerProperties> availableLayers =
        FVulkanRHI::GetAvailableLayers();

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

void FVulkanInstance::Init(GLFWwindow* window)
{
    assert(window != nullptr);

    CreateInstance();
    CreateSurface(window);
    SelectGPU();
}

void FVulkanInstance::SelectGPU()
{
    assert(instance != VK_NULL_HANDLE);
    const auto gpus = GetGPUs();

    // Simplest solution
    for (const FVulkanGPUCreateParam& gpu : gpus) {
        if (gpu.IsValid()) {
            device = std::make_shared<FVulkanGPU>(gpu);
            break;
        }
    }

    if (device == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

void FVulkanInstance::CreateInstance()
{
    if (GE_VALIDATION_LAYERS && !SupportValidationLayer()) {
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

    std::vector<VkExtensionProperties> extensions =
        FVulkanRHI::GetAvailableExtensions();

#if BUILD_DEBUG
    std::cout << "Available extensions:" << std::endl;
    for (const VkExtensionProperties& extension : extensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
    }
#endif

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

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
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    enabledExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    enabledExtensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = enabledExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    createInfo.enabledLayerCount = enabledLayers.size();
    createInfo.ppEnabledLayerNames = enabledLayers.data();

    const VkResult CreateResult =
        vkCreateInstance(&createInfo, nullptr, &instance);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

void FVulkanInstance::CreateSurface(GLFWwindow* window)
{
    const VkResult CreateResult =
        glfwCreateWindowSurface(instance, window, nullptr, &surface);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}
