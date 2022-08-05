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

FVulkanRHI::FVulkanRHI() { commandBuffer = VK_NULL_HANDLE; }

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
        Draw();
    }

    VkDevice _device =
        Instance->GetPhysicalDevice()->GetLogicalDevice()->GetDevice();

    vkDeviceWaitIdle(_device);
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

void FVulkanRHI::Draw()
{
    /*
     * 1. Wait for the previous frame to finish
     * 2. Acquire the next image from the swap chain
     * 3. Create a command buffer for the frame
     * 4. Submit the command buffer to the queue
     * 5. Present the image to the window
     */

    FVulkanDevice* _device = Instance->GetPhysicalDevice()->GetLogicalDevice();

    _device->BeginNextFrame();

    if (commandBuffer == VK_NULL_HANDLE) {
        commandBuffer = _device->CreateCommandBuffer();
    } else {
        vkResetCommandBuffer(commandBuffer, 0);
    }

    _device->Render(commandBuffer);
    _device->Submit(commandBuffer);

    _device->GetSwapChain()->Present();
}
