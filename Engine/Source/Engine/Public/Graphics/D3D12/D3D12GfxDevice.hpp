// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "D3D12CommandListPool.hpp"
#include "D3D12DeferredDeletionQueue.hpp"
#include "D3D12GfxQueue.hpp"
#include "D3D12GfxSync.hpp"
#include "D3D12Include.hpp"
#include "Graphics/GfxDevice.hpp"

class RAVEN_API FD3D12GfxDevice : public IGfxDevice
{
public:
    static constexpr uint32 MaxFramesInFlight = 4;

public:
    FResult Initialize(const FGfxDeviceDesc& InDeviceDesc) override;
    void Deinitialize() override;

    [[nodiscard]] FD3D12GfxQueue& GetQueue(EGfxQueueType Type) const;

    // TResult<FGfxBufferHandle> CreateBuffer(const FGfxBufferDesc& BufferDesc, FStringView DebugName) override;
    // TResult<FGfxTextureHandle> CreateTexture(const FGfxTextureDesc& BufferDesc, FStringView DebugName) override;
    // TResult<FGfxTextureViewHandle> CreateTexture(FGfxTextureHandle TextureHandle, const FGfxTextureViewDesc& TextureViewDesc) override;
    // FGfxSamplerHandle GetOrCreateSampler(const FGfxSamplerDesc& SamplerDesc) override;
    //
    // void DestroyBuffer(FGfxBufferHandle BufferHandle) override;
    // void DestroyTexture(FGfxTextureHandle TextureHandle) override;
    // void DestroyTextureView(FGfxTextureViewHandle TextureViewHandle) override;
    //
    // [[nodiscard]] uint32 GetBindlessIndex(FGfxBufferHandle BufferHandle, EGfxViewType ViewType) const override;
    // [[nodiscard]] uint32 GetBindlessIndex(FGfxTextureHandle TextureHandle, EGfxViewType ViewType) const override;
    // [[nodiscard]] uint32 GetBindlessIndex(FGfxTextureViewHandle TextureViewHandle) const override;
    // [[nodiscard]] uint32 GetBindlessIndex(FGfxSamplerHandle SamplerHandle) const override;
    //
    // [[nodiscard]] TSpan<byte8> Map(FGfxBufferHandle BufferHandle) override;
    // [[nodiscard]] FGfxTextureCopyLayout GetCopyLayout(FGfxTextureHandle TextureHandle, uint32 Subresource) const override;
    //
    // TResult<FGfxShaderHandle> CreateShader(const FGfxShaderDesc& ShaderDesc) override;
    // TResult<FGfxPipelineHandle> CreateGraphicsPipeline(const FGfxGraphicsPipelineDesc& GraphicsPipelineDesc, FStringView DebugName) override;
    // TResult<FGfxPipelineHandle> CreateComputePipeline(const FGfxComputePipelineDesc& ComputePipelineDesc, FStringView DebugName) override;
    // TResult<TUniquePtr<IGfxCommandList>> CreateSwapchain(const FGfxSwapchainDesc& SwapchainDesc) override;
    //
    [[nodiscard]] IGfxCommandList* AcquireCommandList(EGfxQueueType QueueType) override;
    FGfxSyncPoint Submit(const FGfxSubmitDesc& SubmitDesc) override;
    void WaitForSyncPoint(FGfxSyncPoint SyncPoint) override;
    [[nodiscard]] bool8 IsComplete(FGfxSyncPoint SyncPoint) const override;
    void WaitForIdle() override;

    void BeginFrame() override;
    void EndFrame() override;
    [[nodiscard]] uint64 GetFrameIndex() const override;

    [[nodiscard]] uint32 GetFrameSlot() const;
    void DeferRelease(TComPtr<IUnknown> Object, FD3D12DeferredDeletionQueue::FReleaseCallback OnRelease = {});

private:
    FResult AcquireFactory();
    FResult FindSuitableAdapter();
    FResult CreateDevice();
    FResult CheckFeatureSupport();
    FResult CreateQueues();
    void CreateCommandListPools();

    [[nodiscard]] FD3D12QueueFenceValues CaptureLastSubmittedValues() const;
    void WaitForFenceValues(const FD3D12QueueFenceValues& FenceValues);

private:
    bool8 bIsInitialized = false;

    FGfxDeviceDesc DeviceDesc = {};

    TComPtr<IDXGIFactory7> Factory;
    TComPtr<IDXGIAdapter4> Adapter;

    D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_12_0;
    TComPtr<ID3D12Device15> Device;

    FGfxDeviceCapabilities Capabilities = {};

    FD3D12GfxQueue Queues[static_cast<uint8>(EGfxQueueType::Count)];

    uint64 FrameIndex = 0;
    bool8 bIsInFrame = false;
    TArray<FD3D12QueueFenceValues, MaxFramesInFlight> FrameFenceValues = {};
    FD3D12DeferredDeletionQueue DeletionQueue;

    FD3D12CommandListPool CommandListPools[static_cast<uint8>(EGfxQueueType::Count)];
};
