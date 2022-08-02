#include "GameEngine.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Definition.h"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

GameEngine::GameEngine() : physicalDevice(VK_NULL_HANDLE) {}

GameEngine::~GameEngine() {}

void GameEngine::run()
{
    init();
    render();
    cleanup();
}

void GameEngine::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
}

void GameEngine::initRHI()
{
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

void GameEngine::render()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void GameEngine::cleanup()
{
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();

    window = nullptr;
}
