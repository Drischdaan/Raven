// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <atomic>

#include "Core/CoreTypes.hpp"
#include "Core/Memory/SmartPointers.hpp"

class IGfxSwapchain;
class IGfxDevice;
class FWindow;

class RAVEN_API FEngine
{
public:
    void Run();
    void RequestShutdown();

private:
    void Start();
    void Loop();
    void Stop();

    void RenderFrame();

private:
    std::atomic<bool8> bShouldRun = true;

    TSharedPtr<FWindow> Window = nullptr;
    TSharedPtr<IGfxDevice> GfxDevice = nullptr;
    TUniquePtr<IGfxSwapchain> Swapchain = nullptr;
};

extern RAVEN_API FEngine* GEngine;

RAVEN_API void StartupEngine();
[[nodiscard]] RAVEN_API FEngine* GetEngine();
