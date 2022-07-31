#include "GameEngine.h"

#include <iostream>
#include <stdexcept>
#include <vector>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

GameEngine::GameEngine() {}

GameEngine::~GameEngine() {}

void GameEngine::run() {
  init();
  render();
  cleanup();
}

void GameEngine::initWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
}

void GameEngine::initRHI() {
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan Game Engine";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pEngineName = "Game Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         extensions.data());

  std::cout << "Available extensions:" << std::endl;
  for (const VkExtensionProperties &extension : extensions) {
    std::cout << "\t" << extension.extensionName << std::endl;
  }

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;
  createInfo.enabledLayerCount = 0;

  const VkResult CreateResult =
      vkCreateInstance(&createInfo, nullptr, &instance);

  if (CreateResult != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan instance");
  }
}

void GameEngine::init() {
  initWindow();
  initRHI();
}

void GameEngine::render() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

void GameEngine::cleanup() {
  vkDestroyInstance(instance, nullptr);

  glfwDestroyWindow(window);
  glfwTerminate();

  window = nullptr;
}
