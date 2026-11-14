// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Graphics/GfxSwapchain.hpp"
#include "D3D12Include.hpp"
#include "Core/Memory/Containers/Vector.hpp"

class FD3D12GfxDevice;

class RAVEN_API FD3D12GfxSwapchain : public IGfxSwapchain
{
public:
    explicit FD3D12GfxSwapchain(FD3D12GfxDevice& InDevice);
    FD3D12GfxSwapchain(const FD3D12GfxSwapchain&) = delete;
    ~FD3D12GfxSwapchain() override;

    FD3D12GfxSwapchain& operator=(const FD3D12GfxSwapchain&) = delete;

public:
    FResult Initialize(const FGfxSwapchainDesc& SwapchainDesc);

    void WaitForFrameLatency() override;
    TResult<uint32> AcquireNextImage() override;

    [[nodiscard]] FGfxTextureHandle GetBackBuffer(uint32 Index) override;
    [[nodiscard]] EGfxFormat GetFormat() const override;

    FResult Present(bool8 bUseVSync) override;
    FResult Resize(uint32 NewWidth, uint32 NewHeight) override;
    FResult SetHdr(bool8 bUseHdr) override;

private:
    FResult AcquireBackBuffers();
    void ReleaseBackBuffers();

    [[nodiscard]] DXGI_FORMAT GetNativeFormat() const;
    [[nodiscard]] DXGI_COLOR_SPACE_TYPE GetNativeColorSpace() const;
    [[nodiscard]] bool8 IsHdrDisplay() const;

private:
    FD3D12GfxDevice* Device = nullptr;

    TComPtr<IDXGISwapChain4> SwapChain;
    HANDLE FrameLatencyWaitable = nullptr;
    HWND WindowHandle = nullptr;

    TVector<FGfxTextureHandle> BackBuffers;
    uint32 BufferCount = 0;
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 SwapChainFlags = 0;
    bool8 bAllowTearing = false;
    bool8 bIsHdr = false;
};
