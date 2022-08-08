#pragma once

#include <memory>

class FVulkanRHI;

class GameEngine
{
  public:
    GameEngine();
    ~GameEngine();

  public:
    void run();

  private:
    void init();

    void tick();
    void cleanup();

  private:
    std::unique_ptr<FVulkanRHI> RHI;
};
