// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "D3D12Include.hpp"
#include "Graphics/GfxDevice.hpp"

class RAVEN_API FD3D12GfxDevice : public IGfxDevice
{
public:
    FResult Initialize(const FGfxDeviceDesc& InDeviceDesc) override;
    void Deinitialize() override;

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
    // [[nodiscard]] IGfxCommandList* AcquireCommandList(EGfxQueueType QueueType) override;
    // FGfxSyncPoint Submit(const FGfxSubmitDesc& SubmitDesc) override;
    // void WaitForSyncPoint(FGfxSyncPoint SyncPoint) override;
    // [[nodiscard]] bool8 IsComplete(FGfxSyncPoint SyncPoint) const override;
    //
    // void WaitForIdle() override;
    //
    // void BeginFrame() override;
    // void EndFrame() override;
    // [[nodiscard]] uint64 GetFrameIndex() const override;

private:
    FResult AcquireFactory();
    FResult FindSuitableAdapter();
    FResult CreateDevice();

private:
    bool8 bIsInitialized = false;

    FGfxDeviceDesc DeviceDesc = {};

    TComPtr<IDXGIFactory7> Factory;
    TComPtr<IDXGIAdapter4> Adapter;
    TComPtr<ID3D12Device15> Device;
};
