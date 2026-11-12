// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <atomic>

#include "Core/CoreTypes.hpp"
#include "Core/Memory/SmartPointers.hpp"

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

private:
    std::atomic<bool8> bShouldRun = true;

    TSharedPtr<FWindow> Window = nullptr;
    TSharedPtr<IGfxDevice> GfxDevice = nullptr;
};

extern RAVEN_API FEngine* GEngine;

RAVEN_API void StartupEngine();
[[nodiscard]] RAVEN_API FEngine* GetEngine();
