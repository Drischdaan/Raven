// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Engine.hpp"

#include <cassert>
#include <print>
#include <Windows.h>

#include "Core/Memory/Memory.hpp"
#include "Core/Memory/MemoryResource.hpp"
#include "Core/Memory/MemoryTag.hpp"
#include "Core/Memory/Containers/String.hpp"
#include "Graphics/GfxBarriers.hpp"
#include "Graphics/GfxCommandList.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Graphics/GfxSwapchain.hpp"

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

    const FGfxDeviceDesc GfxDeviceDesc = {
        .Backend = EGfxBackend::D3D12,
        .Window = Window,
        .bEnableDebugLayer = true,
        .bEnableGpuValidation = true,
        .bEnableCrashDiagnostics = true,
        .FramesInFlight = 2,
    };
    TResult<TSharedPtr<IGfxDevice>> DeviceResult = CreateGfxDevice(GfxDeviceDesc);
    if (DeviceResult.HasError())
    {
        std::println("Error: {}", DeviceResult.GetError().Message);
        return;
    }
    GfxDevice = DeviceResult.GetValue();

    const FGfxSwapchainDesc SwapchainDesc = {
        .Window = Window,
        .Width = 0,
        .Height = 0,
        .BufferCount = 3,
        .bUseHdr = false,
        .bUseVSync = true,
    };
    TResult<TUniquePtr<IGfxSwapchain>> SwapchainResult = GfxDevice->CreateSwapchain(SwapchainDesc);
    if (SwapchainResult.HasError())
    {
        std::println("Error: {}", SwapchainResult.GetError().Message);
        RequestShutdown();
        return;
    }
    Swapchain = std::move(SwapchainResult).GetValue();
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

        const FWindowState& State = Window->GetWindowState();
        if (State.ClientWidth == 0 || State.ClientHeight == 0)
        {
            Sleep(16);
            continue;
        }
        FResult ResizeResult = Swapchain->Resize(State.ClientWidth, State.ClientHeight); // no-op if unchanged
        if (ResizeResult.HasError())
        {
            std::println("Error: {}", ResizeResult.GetError().Message);
            RequestShutdown();
            continue;
        }
        RenderFrame();
    }
}

void FEngine::Stop()
{
    Swapchain.reset();
    GfxDevice.reset();
    Window.reset();
}

void FEngine::RenderFrame()
{
    Swapchain->WaitForFrameLatency();
    GfxDevice->BeginFrame();

    TResult<uint32> ImageIndex = Swapchain->AcquireNextImage();
    if (ImageIndex.HasError())
    {
        GfxDevice->EndFrame();
        return;
    }
    const FGfxTextureHandle BackBuffer = Swapchain->GetBackBuffer(ImageIndex.GetValue());

    IGfxCommandList* CommandList = GfxDevice->AcquireCommandList(EGfxQueueType::Graphics);
    CommandList->BeginEvent("Clear");

    const FGfxTextureBarrier ToRenderTarget = {
        .TextureHandle = BackBuffer,
        .SyncBefore = EGfxBarrierSync::None,
        .SyncAfter = EGfxBarrierSync::RenderTarget,
        .AccessBefore = EGfxBarrierAccess::NoAccess,
        .AccessAfter = EGfxBarrierAccess::RenderTarget,
        .LayoutBefore = EGfxTextureLayout::Undefined,
        .LayoutAfter = EGfxTextureLayout::RenderTarget,
    };
    CommandList->Barrier({}, {}, {&ToRenderTarget, 1});

    const FGfxRenderTargetAttachment ColorTarget = {
        .TextureHandle = BackBuffer,
        .LoadOp = EGfxLoadOp::Clear,
        .StoreOp = EGfxStoreOp::Store,
        .ClearColor = {0.10f, 0.20f, 0.40f, 1.0f},
    };
    CommandList->BeginRendering({.RenderTargets = {&ColorTarget, 1}});
    CommandList->EndRendering();

    const FGfxTextureBarrier ToPresent = {
        .TextureHandle = BackBuffer,
        .SyncBefore = EGfxBarrierSync::RenderTarget,
        .SyncAfter = EGfxBarrierSync::None,
        .AccessBefore = EGfxBarrierAccess::RenderTarget,
        .AccessAfter = EGfxBarrierAccess::NoAccess,
        .LayoutBefore = EGfxTextureLayout::RenderTarget,
        .LayoutAfter = EGfxTextureLayout::Present,
    };
    CommandList->Barrier({}, {}, {&ToPresent, 1});
    CommandList->EndEvent();

    IGfxCommandList* const CommandLists[] = {CommandList};
    GfxDevice->Submit({
        .QueueType = EGfxQueueType::Graphics,
        .CommandLists = CommandLists,
        .WaitForAcquire = Swapchain.get(),
        .SignalForPresent = Swapchain.get(),
    });

    const FResult PresentResult = Swapchain->Present(true);
    if (PresentResult.HasError())
    {
        std::println("Error: {}", PresentResult.GetError().Message);
        RequestShutdown();
    }

    GfxDevice->EndFrame();
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
