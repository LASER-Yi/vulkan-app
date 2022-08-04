#include "GameEngine.h"

#include <memory>

#include "Definition.h"
#include "VulkanRHI/VulkanRHI.h"

GameEngine::GameEngine() {}

GameEngine::~GameEngine() {}

void GameEngine::run()
{
    init();
    tick();
    cleanup();
}

void GameEngine::init()
{
    RHI = std::make_unique<FVulkanRHI>();

    RHI->Init();
}

void GameEngine::tick() { RHI->Render(); }

void GameEngine::cleanup() { RHI->Destroy(); }
