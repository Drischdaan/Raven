// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Engine.hpp"

#include <cassert>
#include <print>
#include <Windows.h>

#include "Core/CoreLog.hpp"
#include "Core/Logging/Log.hpp"
#include "Core/Memory/Memory.hpp"
#include "Core/Memory/MemoryResource.hpp"
#include "Core/Memory/MemoryTag.hpp"
#include "Platform/Window.hpp"

FEngine* GEngine = nullptr;

void FEngine::Run()
{
    const FResult Result = Start();
    if (Result.HasError())
    {
        RAVEN_LOG_ERROR(LogCore, Result.GetError());
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

    FRendererDesc RendererDesc = {.Window = Window};
#if defined(RAVEN_MODE_DEBUG)
    RendererDesc.bEnableDebugLayer = true;
    RendererDesc.bShaderDebugInfo = true;
#endif
    Renderer = MakeUnique<FRenderer>();
    RESULT_CHECK(Renderer->Initialize(RendererDesc));
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

        const FResult RenderResult = Renderer->Render(FRenderView{});
        if (RenderResult.HasError())
        {
            RAVEN_LOG_ERROR(LogCore, RenderResult.GetError());
            RequestShutdown();
        }
    }
}

void FEngine::Stop()
{
    Renderer.reset();
    Window.reset();
}

void StartupEngine()
{
    if (GEngine != nullptr)
    {
        return;
    }
    FLog::Initialize();
    RAVEN_LOG(LogCore, Info, "Raven starting ({})", RAVEN_MODE);
    RAVEN_MEMORY_SCOPE(Core);
    FMemory::Initialize();
    GEngine = FMemory::NewObject<FEngine>();
    GEngine->Run();
    FMemory::DeleteObject(GEngine);
    FMemory::Deinitialize();
    FLog::Shutdown();
}

FEngine* GetEngine()
{
    assert(GEngine != nullptr);
    return GEngine;
}
