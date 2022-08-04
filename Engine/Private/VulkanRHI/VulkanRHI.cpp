#include "VulkanRHI/VulkanRHI.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "GLFW/glfw3.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanDevice.h"
#include "VulkanRHI/VulkanShader.h"
#include "VulkanRHI/VulkanSwapChain.h"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

FVulkanRHI::FVulkanRHI() {}

void FVulkanRHI::Init()
{
    CreateWindow();

    Instance = std::make_unique<FVulkanInstance>(window);
}

void FVulkanRHI::Destroy()
{

    Instance.reset();

    glfwDestroyWindow(window);
    window = nullptr;
}

void FVulkanRHI::Render()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

FVulkanInstance* FVulkanRHI::GetInstance() const { return Instance.get(); }

std::vector<VkExtensionProperties> FVulkanRHI::GetAvailableExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           extensions.data());

    return std::move(extensions);
}

std::vector<VkLayerProperties> FVulkanRHI::GetAvailableLayers()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    return std::move(layers);
}

void FVulkanRHI::CreateWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
}
