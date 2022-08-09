#include "VulkanRHI/VulkanRHI.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>

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

    auto vk_device =
        Instance->GetPhysicalDevice()->GetLogicalDevice()->GetDevice();

    vk_device.waitIdle();
}

FVulkanInstance* FVulkanRHI::GetInstance() const { return Instance.get(); }

std::vector<vk::ExtensionProperties> FVulkanRHI::GetAvailableExtensions()
{
    return vk::enumerateInstanceExtensionProperties();
}

std::vector<vk::LayerProperties> FVulkanRHI::GetAvailableLayers()
{
    return vk::enumerateInstanceLayerProperties();
}

void FVulkanRHI::CreateWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, OnFramebufferResize);
}

void FVulkanRHI::OnFramebufferResize(GLFWwindow* window, int width, int height)
{
    // silent unused parameter warning
    (void)width;
    (void)height;

    auto rhi = reinterpret_cast<FVulkanRHI*>(glfwGetWindowUserPointer(window));

    rhi->GetInstance()
        ->GetPhysicalDevice()
        ->GetLogicalDevice()
        ->GetSwapChain()
        ->SetNeedResize();
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

    if (commandBuffer == nullptr) {
        commandBuffer =
            std::make_shared<vk::CommandBuffer>(_device->CreateCommandBuffer());
    } else {
        commandBuffer->reset();
    }

    _device->Render(commandBuffer.get());
    _device->Submit(commandBuffer.get());

    _device->GetSwapChain()->Present();
}
