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

    _device->WaitRenderFinished();

    const uint32_t imageIndex = _device->GetSwapChain()->GetNextImageIndex();

    if (commandBuffer == VK_NULL_HANDLE) {
        commandBuffer = _device->CreateCommandBuffer();
    } else {
        vkResetCommandBuffer(commandBuffer, 0);
    }

    _device->Submit(commandBuffer, imageIndex);

    // TODO: Move to somewhere else
    VkSubmitInfo submitInfo = {};
    ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        _device->GetSwapChain()->GetImageAvailableSemaphore()};

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {
        _device->GetSwapChain()->GetRenderFinishedSemaphore()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkQueue graphicsQueue = _device->GetGraphicsQueue();
    VkFence renderFence = _device->TEMP_GetRenderFence();
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderFence) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    // Presentation
    VkPresentInfoKHR presentInfo = {};
    ZeroVulkanStruct(presentInfo, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {
        _device->GetSwapChain()->TEMP_GetSwapChain()};

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(graphicsQueue, &presentInfo);
}
