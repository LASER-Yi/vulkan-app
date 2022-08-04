#include "VulkanRHI/VulkanRHI.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vulkan/vulkan_core.h>

#include "GLFW/glfw3.h"
#include "VulkanRHI/VulkanDevice.h"
#include "VulkanRHI/VulkanShader.h"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

FVulkanRHI::FVulkanRHI() {}

void FVulkanRHI::Init()
{
    CreateWindow();

    Instance = std::make_unique<FVulkanInstance>(window);

    CreatePipeline();
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

void FVulkanRHI::CreatePipeline()
{
    FVulkanDevice* device = Instance->GetPhysicalDevice()->GetLogicalDevice();

    auto VertShader = device->CreateShader("shaders/triangle.vert.spv",
                                           VK_SHADER_STAGE_VERTEX_BIT);
    auto FragShader = device->CreateShader("shaders/triangle.frag.spv",
                                           VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        VertShader->CreatePipelineStage(), FragShader->CreatePipelineStage()};
}
