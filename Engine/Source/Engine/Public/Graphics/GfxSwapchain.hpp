// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "GfxTypes.hpp"
#include "Core/Memory/SmartPointers.hpp"
#include "Core/Utility/Result.hpp"

class FWindow;

struct RAVEN_API FGfxSwapchainDesc
{
    TSharedPtr<FWindow> Window = nullptr;
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 BufferCount = 3;
    bool8 bUseHdr = false;
    bool8 bUseVSync = true;
};

class RAVEN_API IGfxSwapchain
{
public:
    virtual ~IGfxSwapchain() = default;

public:
    virtual void WaitForFrameLatency() = 0;

    virtual TResult<uint32> AcquireNextImage() = 0;

    [[nodiscard]] virtual FGfxTextureHandle GetBackBuffer(uint32 Index) = 0;
    [[nodiscard]] virtual EGfxFormat GetFormat() const = 0;

    virtual FResult Present(bool8 bUseVSync) = 0;
    virtual FResult Resize(uint32 Width, uint32 Height) = 0;
    virtual FResult SetHdr(bool8 bUseHdr) = 0;
};
