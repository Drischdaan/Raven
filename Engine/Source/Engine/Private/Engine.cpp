// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Engine.hpp"

#include <cassert>
#include <print>
#include <Windows.h>

#include "Core/Memory/Memory.hpp"

#include "Platform/Window.hpp"

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
    FWindowSettings WindowSettings = {};
    Window = MakeShared<FWindow>(WindowSettings);
    Window->Show();
}

void FEngine::Loop()
{
    while (bShouldRun)
    {
        if (Window->GetWindowState().bIsClosed)
        {
            RequestShutdown();
        }

        MSG Message;
        while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }
}

void FEngine::Stop()
{
    Window.reset();
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
