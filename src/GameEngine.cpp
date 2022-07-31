#include "GameEngine.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

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
    pickPhysicalDevice();
    createLogicalDevice();

    assert(instance != VK_NULL_HANDLE);
    assert(physicalDevice != VK_NULL_HANDLE);
    assert(logicalDevice != VK_NULL_HANDLE);
    assert(graphicsQueue != VK_NULL_HANDLE);
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
    vkDestroyInstance(instance, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();

    window = nullptr;
}
