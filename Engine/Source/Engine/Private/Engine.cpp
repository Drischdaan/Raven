// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Engine.hpp"

#include <cassert>
#include <print>
#include <Windows.h>

#include "Core/Memory/Memory.hpp"
#include "Core/Memory/MemoryResource.hpp"
#include "Core/Memory/MemoryTag.hpp"
#include "Graphics/GfxDevice.hpp"

#include "Platform/Window.hpp"

FEngine* GEngine = nullptr;

void FEngine::Run()
{
    const FResult Result = Start();
    if (Result.HasError())
    {
        std::println("Error: {}", Result.GetError().Message);
        Stop();
        return;
    }
    Loop();
    Stop();
}

void FEngine::RequestShutdown()
{
    bShouldRun = false;
}

FResult FEngine::Start()
{
    FWindowSettings WindowSettings = {};
    Window = MakeShared<FWindow>(WindowSettings);
    Window->Show();
    return RESULT_OK_EMPTY();
}

void FEngine::Loop()
{
    while (bShouldRun)
    {
        if (Window->GetWindowState().bIsClosed)
        {
            RequestShutdown();
            continue;
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
    RAVEN_MEMORY_SCOPE(Core);
    FMemory::Initialize();
    GEngine = FMemory::NewObject<FEngine>();
    GEngine->Run();
    FMemory::DeleteObject(GEngine);
    FMemory::Deinitialize();
}

FEngine* GetEngine()
{
    assert(GEngine != nullptr);
    return GEngine;
}
