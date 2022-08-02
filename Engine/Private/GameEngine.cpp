#include "GameEngine.h"

#include <memory>

#include "Definition.h"

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
    RHI = std::make_shared<FVulkanRHI>();

    RHI->Init();
}

void GameEngine::tick() { RHI->Render(); }

void GameEngine::cleanup() {}
