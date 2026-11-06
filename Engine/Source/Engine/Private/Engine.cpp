// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Engine.hpp"

#include <cassert>
#include <print>
#include <Windows.h>

#include "Core/Memory/Memory.hpp"

FEngine* GEngine = nullptr;

void FEngine::Run()
{
    Start();
    Loop();
    Stop();
}

void FEngine::RequestShutdown()
{
    bShouldRun = false;
}

void FEngine::Start()
{
}

void FEngine::Loop()
{
    while (bShouldRun)
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            RequestShutdown();
        }
    }
}

void FEngine::Stop()
{
}


void StartupEngine()
{
    if (GEngine != nullptr)
    {
        return;
    }
    GEngine = FMemory::NewObject<FEngine>();
    GEngine->Run();
    FMemory::DeleteObject(GEngine);
}

FEngine* GetEngine()
{
    assert(GEngine != nullptr);
    return GEngine;
}
