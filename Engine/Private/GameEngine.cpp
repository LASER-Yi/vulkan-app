#include "GameEngine.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Definition.h"

GameEngine::GameEngine() : physicalDevice(VK_NULL_HANDLE) {}

GameEngine::~GameEngine() {}

void GameEngine::run()
{
    init();
    tick();
    cleanup();
}

void GameEngine::initWindow() {}

void GameEngine::initRHI()
{
    RHI = std::make_shared<FVulkanRHI>();

    RHI->Init();

    return;
    createInstance();
    assert(instance != VK_NULL_HANDLE);

    createWindowSurface();
    assert(surface != VK_NULL_HANDLE);

    pickPhysicalDevice();
    assert(physicalDevice != VK_NULL_HANDLE);

    createLogicalDevice();
    assert(logicalDevice != VK_NULL_HANDLE);
    assert(graphicsQueue != VK_NULL_HANDLE);

    createSwapChain();
    assert(swapChain != VK_NULL_HANDLE);
}

void GameEngine::createWindowSurface()
{
    const VkResult CreateResult =
        glfwCreateWindowSurface(instance, window, nullptr, &surface);

    if (CreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void GameEngine::init()
{
    initWindow();
    initRHI();
}

void GameEngine::tick() { RHI->Render(); }

void GameEngine::cleanup()
{
    return;
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();

    window = nullptr;
}
