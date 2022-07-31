#include "GameEngine.h"

#include <iostream>
#include <stdexcept>
#include <vector>

#include "Definition.h"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

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
  if (GE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
    throw std::runtime_error("Validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan Game Engine";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pEngineName = "Game Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.apiVersion = VK_API_VERSION_1_1;

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

#if GE_VALIDATION_LAYERS
  createInfo.enabledExtensionCount = validationLayers.size();
  createInfo.ppEnabledExtensionNames = validationLayers.data();
#else
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = nullptr;
#endif

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

bool GameEngine::checkValidationLayerSupport() {
#if PLATFORM_MACOS
  return true;
#else
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
  for (const char *layerName : validationLayers) {
    bool layerFound = false;
    for (const VkLayerProperties &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }
    if (!layerFound) {
      return false;
    }
  }
  return false;
#endif
}
