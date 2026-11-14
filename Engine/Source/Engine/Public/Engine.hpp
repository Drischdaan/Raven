// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <atomic>

#include "Core/CoreTypes.hpp"
#include "Core/Memory/SmartPointers.hpp"
#include "Core/Utility/Result.hpp"
#include "Renderer/Renderer.hpp"

class IGfxSwapchain;
class IGfxDevice;
class FWindow;

class RAVEN_API FEngine
{
public:
    void Run();
    void RequestShutdown();

private:
    FResult Start();
    void Loop();
    void Stop();

private:
    std::atomic<bool8> bShouldRun = true;

    TSharedPtr<FWindow> Window = nullptr;
    TUniquePtr<FRenderer> Renderer = nullptr;
};

extern RAVEN_API FEngine* GEngine;

RAVEN_API void StartupEngine();
[[nodiscard]] RAVEN_API FEngine* GetEngine();
