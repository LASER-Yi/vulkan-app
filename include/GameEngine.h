#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class GameEngine {
public:
  GameEngine();
  ~GameEngine();

public:
  void run();

private:
  void init();
  void initWindow();
  void initRHI();

  void render();
  void cleanup();

private:
  GLFWwindow *window;
  VkInstance instance;
};
