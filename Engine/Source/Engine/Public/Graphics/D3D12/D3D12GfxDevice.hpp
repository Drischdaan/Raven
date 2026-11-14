// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "D3D12Bindless.hpp"
#include "D3D12CommandListPool.hpp"
#include "D3D12DeferredDeletionQueue.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12GfxQueue.hpp"
#include "D3D12GfxResources.hpp"
#include "D3D12GfxSync.hpp"
#include "D3D12Include.hpp"
#include "D3D12TransientAllocator.hpp"
#include "Core/Memory/Containers/Map.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Graphics/GfxResourcePool.hpp"

struct RAVEN_API FGfxSamplerDescHasher
{
    [[nodiscard]] uint64 operator()(const FGfxSamplerDesc& Desc) const noexcept;
};

class RAVEN_API FD3D12GfxDevice : public IGfxDevice
{
public:
    static constexpr uint32 MaxFramesInFlight = 4;

public:
    ~FD3D12GfxDevice() override;

    FResult Initialize(const FGfxDeviceDesc& InDeviceDesc) override;
    void Deinitialize() override;

    [[nodiscard]] FD3D12GfxQueue& GetQueue(EGfxQueueType Type) const;

    TResult<FGfxBufferHandle> CreateBuffer(const FGfxBufferDesc& BufferDesc, FStringView DebugName) override;
    TResult<FGfxTextureHandle> CreateTexture(const FGfxTextureDesc& TextureDesc, FStringView DebugName) override;
    TResult<FGfxTextureViewHandle> CreateTextureView(FGfxTextureHandle TextureHandle, const FGfxTextureViewDesc& TextureViewDesc) override;
    TResult<FGfxSamplerHandle> GetOrCreateSampler(const FGfxSamplerDesc& SamplerDesc) override;

    void DestroyBuffer(FGfxBufferHandle BufferHandle) override;
    void DestroyTexture(FGfxTextureHandle TextureHandle) override;
    void DestroyTextureView(FGfxTextureViewHandle TextureViewHandle) override;

    [[nodiscard]] uint32 GetBindlessIndex(FGfxBufferHandle BufferHandle, EGfxViewType ViewType) const override;
    [[nodiscard]] uint32 GetBindlessIndex(FGfxTextureHandle TextureHandle, EGfxViewType ViewType) const override;
    [[nodiscard]] uint32 GetBindlessIndex(FGfxTextureViewHandle TextureViewHandle) const override;
    [[nodiscard]] uint32 GetBindlessIndex(FGfxSamplerHandle SamplerHandle) const override;
    [[nodiscard]] FD3D12DescriptorHeap& GetDescriptorHeap(ED3D12DescriptorHeapType HeapType);
    [[nodiscard]] ID3D12RootSignature* GetGlobalRootSignature() const;

    [[nodiscard]] FD3D12Buffer* GetBuffer(FGfxBufferHandle BufferHandle);
    [[nodiscard]] FD3D12Texture* GetTexture(FGfxTextureHandle TextureHandle);
    [[nodiscard]] FD3D12TextureView* GetTextureView(FGfxTextureViewHandle TextureViewHandle);
    [[nodiscard]] FD3D12Pipeline* GetPipeline(FGfxPipelineHandle PipelineHandle);

    void FreeDescriptorDeferred(ED3D12DescriptorHeapType HeapType, uint32 Index);

    [[nodiscard]] TSpan<byte8> Map(FGfxBufferHandle BufferHandle) override;
    [[nodiscard]] FGfxTextureCopyLayout GetCopyLayout(FGfxTextureHandle TextureHandle, uint32 Subresource) const override;

    // TResult<FGfxShaderHandle> CreateShader(const FGfxShaderDesc& ShaderDesc) override;
    // TResult<FGfxPipelineHandle> CreateGraphicsPipeline(const FGfxGraphicsPipelineDesc& GraphicsPipelineDesc, FStringView DebugName) override;
    // TResult<FGfxPipelineHandle> CreateComputePipeline(const FGfxComputePipelineDesc& ComputePipelineDesc, FStringView DebugName) override;

    TResult<TUniquePtr<IGfxSwapchain>> CreateSwapchain(const FGfxSwapchainDesc& SwapchainDesc) override;

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

    [[nodiscard]] TResult<FGfxTextureHandle> RegisterSwapchainImage(TComPtr<ID3D12Resource> Resource, EGfxFormat Format, FStringView DebugName);
    void UnregisterSwapchainImage(FGfxTextureHandle TextureHandle);

    [[nodiscard]] IDXGIFactory7* GetFactory() const;
    [[nodiscard]] const FGfxDeviceCapabilities& GetCapabilities() const;
    [[nodiscard]] uint32 GetFramesInFlight() const;
    [[nodiscard]] ID3D12Device15* GetNativeDevice() const;
    [[nodiscard]] FGfxTransientAllocation AllocateTransient(uint64 Size, uint64 Alignment);

private:
    FResult AcquireFactory();
    FResult FindSuitableAdapter();
    FResult CreateDevice();
    FResult CheckFeatureSupport();
    FResult CreateQueues();
    void CreateCommandListPools();
    FResult CreateDescriptorHeaps();
    void WriteDefaultDescriptors();
    FResult CreateGlobalRootSignature();

    [[nodiscard]] FD3D12QueueFenceValues CaptureLastSubmittedValues() const;
    void WaitForFenceValues(const FD3D12QueueFenceValues& FenceValues);

    void ReportAndReleaseLeakedResources();

    FResult CreateBufferViews(FD3D12Buffer& Buffer);
    FResult CreateTextureDefaultViews(FD3D12Texture& Texture);
    void FreeDescriptorsNow(uint32 SrvIndex, uint32 UavIndex);

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

    FD3D12DescriptorHeap DescriptorHeaps[static_cast<uint8>(ED3D12DescriptorHeapType::Count)];
    TComPtr<ID3D12RootSignature> GlobalRootSignature;

    TGfxResourcePool<FGfxBufferHandle, FD3D12Buffer> Buffers;
    TGfxResourcePool<FGfxTextureHandle, FD3D12Texture> Textures;
    TGfxResourcePool<FGfxTextureViewHandle, FD3D12TextureView> TextureViews;
    TGfxResourcePool<FGfxSamplerHandle, FD3D12Sampler, 2048> Samplers;
    TGfxResourcePool<FGfxShaderHandle, FD3D12Shader> Shaders;
    TGfxResourcePool<FGfxPipelineHandle, FD3D12Pipeline> Pipelines;

    FD3D12TransientAllocator TransientAllocator;

    std::mutex SamplerCacheMutex;
    TUnorderedMap<FGfxSamplerDesc, FGfxSamplerHandle, FGfxSamplerDescHasher> SamplerCache;
};
