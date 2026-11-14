// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12Swapchain.hpp"

#include "Graphics/D3D12/D3D12GfxDevice.hpp"
#include "Platform/Window.hpp"

FD3D12GfxSwapchain::FD3D12GfxSwapchain(FD3D12GfxDevice& InDevice)
    : Device(&InDevice)
{
}

FD3D12GfxSwapchain::~FD3D12GfxSwapchain()
{
    if (SwapChain)
    {
        Device->WaitForIdle();
    }
    ReleaseBackBuffers();
    if (FrameLatencyWaitable != nullptr)
    {
        CloseHandle(FrameLatencyWaitable);
        FrameLatencyWaitable = nullptr;
    }
    SwapChain.Reset();
}

FResult FD3D12GfxSwapchain::Initialize(const FGfxSwapchainDesc& SwapchainDesc)
{
    if (SwapchainDesc.Window == nullptr)
    {
        return RESULT_ERROR("Swapchain needs a window");
    }
    if (SwapchainDesc.BufferCount < 2 || SwapchainDesc.BufferCount > DXGI_MAX_SWAP_CHAIN_BUFFERS)
    {
        return RESULT_ERROR("Swapchain buffer count must be between 2 and 16");
    }

    WindowHandle = SwapchainDesc.Window->GetNativeHandle();
    BufferCount = SwapchainDesc.BufferCount;
    bAllowTearing = Device->GetCapabilities().bSupportsTearing;
    bIsHdr = false;

    SwapChainFlags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    if (bAllowTearing)
    {
        SwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    const DXGI_SWAP_CHAIN_DESC1 NativeDesc = {
        .Width = SwapchainDesc.Width,
        .Height = SwapchainDesc.Height,
        .Format = GetNativeFormat(),
        .Stereo = FALSE,
        .SampleDesc = {.Count = 1, .Quality = 0},
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = BufferCount,
        .Scaling = DXGI_SCALING_STRETCH,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
        .Flags = SwapChainFlags,
    };

    IDXGIFactory7* Factory = Device->GetFactory();
    ID3D12CommandQueue* GraphicsQueue = Device->GetQueue(EGfxQueueType::Graphics).GetNative();

    TComPtr<IDXGISwapChain1> SwapChain1;
    RESULT_TRY_PLATFORM_CALL(Factory->CreateSwapChainForHwnd(GraphicsQueue, WindowHandle, &NativeDesc, nullptr, nullptr, &SwapChain1), "Failed to create swapchain")
    RESULT_TRY_PLATFORM_CALL(SwapChain1.As(&SwapChain), "Swapchain doesn't support IDXGISwapChain4")
    RESULT_TRY_PLATFORM_CALL(Factory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_ALT_ENTER), "Failed to disable Alt+Enter")

    RESULT_TRY_PLATFORM_CALL(SwapChain->SetMaximumFrameLatency(Device->GetFramesInFlight()), "Failed to set the maximum frame latency")
    FrameLatencyWaitable = SwapChain->GetFrameLatencyWaitableObject();

    DXGI_SWAP_CHAIN_DESC1 CreatedDesc = {};
    RESULT_TRY_PLATFORM_CALL(SwapChain->GetDesc1(&CreatedDesc), "Failed to query the swapchain")
    Width = CreatedDesc.Width;
    Height = CreatedDesc.Height;

    RESULT_TRY_PLATFORM_CALL(SwapChain->SetColorSpace1(GetNativeColorSpace()), "Failed to set the swapchain color space")
    RESULT_CHECK(AcquireBackBuffers())

    if (SwapchainDesc.bUseHdr)
    {
        const FResult HdrResult = SetHdr(true);
        if (HdrResult.HasError())
        {
            RAVEN_LOG(LogD3D12, Warning, "HDR unavailable, using SDR: {}", HdrResult.GetError().Message);
        }
    }
    return RESULT_OK_EMPTY();
}

void FD3D12GfxSwapchain::WaitForFrameLatency()
{
    if (FrameLatencyWaitable != nullptr)
    {
        WaitForSingleObjectEx(FrameLatencyWaitable, 1000, TRUE);
    }
}

TResult<uint32> FD3D12GfxSwapchain::AcquireNextImage()
{
    return RESULT_OK(SwapChain->GetCurrentBackBufferIndex());
}

FGfxTextureHandle FD3D12GfxSwapchain::GetBackBuffer(const uint32 Index)
{
    return Index < BackBuffers.size() ? BackBuffers[Index] : FGfxTextureHandle{};
}

EGfxFormat FD3D12GfxSwapchain::GetFormat() const
{
    return bIsHdr ? EGfxFormat::RGB10A2_UNORM : EGfxFormat::RGBA8_SRGB;
}

FResult FD3D12GfxSwapchain::Present(const bool8 bUseVSync)
{
    const uint32 SyncInterval = bUseVSync ? 1 : 0;
    const uint32 PresentFlags = (!bUseVSync && bAllowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

    const HRESULT Result = Device->GetQueue(EGfxQueueType::Graphics).Present(SwapChain.Get(), SyncInterval, PresentFlags);
    if (Result == DXGI_ERROR_DEVICE_REMOVED || Result == DXGI_ERROR_DEVICE_RESET)
    {
        return RESULT_PLATFORM_ERROR(Device->GetNativeDevice()->GetDeviceRemovedReason(), "The GPU device was removed during present");
    }
    if (FAILED(Result))
    {
        return RESULT_PLATFORM_ERROR(Result, "Present failed");
    }
    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxSwapchain::Resize(const uint32 NewWidth, const uint32 NewHeight)
{
    if (NewWidth == 0 || NewHeight == 0)
    {
        return RESULT_OK_EMPTY();
    }
    if (NewWidth == Width && NewHeight == Height)
    {
        return RESULT_OK_EMPTY();
    }

    Device->WaitForIdle();
    ReleaseBackBuffers();

    RESULT_TRY_PLATFORM_CALL(SwapChain->ResizeBuffers(BufferCount, NewWidth, NewHeight, GetNativeFormat(), SwapChainFlags), "Failed to resize the swapchain")
    Width = NewWidth;
    Height = NewHeight;
    return AcquireBackBuffers();
}

FResult FD3D12GfxSwapchain::SetHdr(const bool8 bUseHdr)
{
    if (bUseHdr == bIsHdr)
    {
        return RESULT_OK_EMPTY();
    }
    if (bUseHdr && !IsHdrDisplay())
    {
        return RESULT_ERROR("The display isn't in HDR mode (enable \"Use HDR\" in the Windows display settings)");
    }

    Device->WaitForIdle();
    ReleaseBackBuffers();

    bIsHdr = bUseHdr;
    const HRESULT ResizeResult = SwapChain->ResizeBuffers(BufferCount, Width, Height, GetNativeFormat(), SwapChainFlags);
    if (FAILED(ResizeResult))
    {
        bIsHdr = !bUseHdr;
        RESULT_CHECK(AcquireBackBuffers())
        return RESULT_PLATFORM_ERROR(ResizeResult, "Failed to change the swapchain format");
    }

    uint32 ColorSpaceSupport = 0;
    const HRESULT SupportResult = SwapChain->CheckColorSpaceSupport(GetNativeColorSpace(), &ColorSpaceSupport);
    if (FAILED(SupportResult) || (ColorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) == 0)
    {
        RESULT_CHECK(SetHdr(!bUseHdr))
        return RESULT_ERROR("The swapchain can't present in the requested color space");
    }

    RESULT_TRY_PLATFORM_CALL(SwapChain->SetColorSpace1(GetNativeColorSpace()), "Failed to set the swapchain color space")
    return AcquireBackBuffers();
}

FResult FD3D12GfxSwapchain::AcquireBackBuffers()
{
    BackBuffers.clear();
    for (uint32 Index = 0; Index < BufferCount; ++Index)
    {
        TComPtr<ID3D12Resource> Buffer;
        RESULT_TRY_PLATFORM_CALL_CLEANUP(SwapChain->GetBuffer(Index, IID_PPV_ARGS(&Buffer)), ReleaseBackBuffers();, "Failed to get a swapchain buffer")

        const FString DebugName = String::Format("SwapchainBackBuffer{}", Index);
        TResult<FGfxTextureHandle> Handle = Device->RegisterSwapchainImage(std::move(Buffer), GetFormat(), DebugName);
        if (Handle.HasError())
        {
            ReleaseBackBuffers();
            return std::unexpected(Handle.GetError());
        }
        BackBuffers.push_back(Handle.GetValue());
    }
    return RESULT_OK_EMPTY();
}

void FD3D12GfxSwapchain::ReleaseBackBuffers()
{
    for (const FGfxTextureHandle BackBuffer : BackBuffers)
    {
        Device->UnregisterSwapchainImage(BackBuffer);
    }
    BackBuffers.clear();
}

DXGI_FORMAT FD3D12GfxSwapchain::GetNativeFormat() const
{
    return bIsHdr ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
}

DXGI_COLOR_SPACE_TYPE FD3D12GfxSwapchain::GetNativeColorSpace() const
{
    return bIsHdr ? DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 : DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
}

bool8 FD3D12GfxSwapchain::IsHdrDisplay() const
{
    TComPtr<IDXGIOutput> Output;
    if (FAILED(SwapChain->GetContainingOutput(&Output)))
    {
        return false;
    }
    TComPtr<IDXGIOutput6> Output6;
    if (FAILED(Output.As(&Output6)))
    {
        return false;
    }
    DXGI_OUTPUT_DESC1 OutputDesc = {};
    if (FAILED(Output6->GetDesc1(&OutputDesc)))
    {
        return false;
    }
    return OutputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
}
