#include <memory>

#include "VulkanRHI/VulkanRHI.h"

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
    std::shared_ptr<FVulkanRHI> RHI;
};
