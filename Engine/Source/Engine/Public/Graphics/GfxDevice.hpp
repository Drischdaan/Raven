// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "GfxResources.hpp"
#include "GfxTypes.hpp"
#include "Core/Memory/SmartPointers.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Memory/Containers/String.hpp"
#include "Core/Utility/Result.hpp"

class FWindow;
struct FGfxShaderDesc;
struct FGfxGraphicsPipelineDesc;
struct FGfxComputePipelineDesc;
struct FGfxSwapchainDesc;
class IGfxCommandList;

struct FGfxSubmitDesc
{
    EGfxQueueType QueueType;
    TSpan<IGfxCommandList* const> CommandLists;
    TSpan<const FGfxSyncPoint> WaitFor;
    IGfxCommandList* WaitForAcquire = nullptr;
    IGfxCommandList* SignalForPresent = nullptr;
};

struct RAVEN_API FGfxDeviceDesc
{
    EGfxBackend Backend = EGfxBackend::D3D12;
    TSharedPtr<FWindow> Window = nullptr;
    bool8 bEnableDebugLayer = false;
    bool8 bEnableGpuValidation = false;
    bool8 bEnableCrashDiagnostics = true;
    uint32 FramesInFlight = 2;
};

struct RAVEN_API FGfxDeviceCapabilities
{
    FString AdapterName;
    uint64 DedicatedVideoMemory = 0;
    bool8 bSupportsRayTracing = false;
    bool8 bSupportsMeshShaders = false;
    bool8 bSupportsGpuUploadHeap = false;
    bool8 bSupportsTearing = false;
    bool8 bSupportsVariableRateShading = false;
};

class RAVEN_API IGfxDevice
{
public:
    virtual ~IGfxDevice() = default;

public:
    virtual FResult Initialize(const FGfxDeviceDesc& InDeviceDesc) = 0;
    virtual void Deinitialize() = 0;

    // virtual TResult<FGfxBufferHandle> CreateBuffer(const FGfxBufferDesc& BufferDesc, FStringView DebugName = "UnnamedBuffer") = 0;
    // virtual TResult<FGfxTextureHandle> CreateTexture(const FGfxTextureDesc& BufferDesc, FStringView DebugName = "UnnamedTexture") = 0;
    // virtual TResult<FGfxTextureViewHandle> CreateTexture(FGfxTextureHandle TextureHandle, const FGfxTextureViewDesc& TextureViewDesc) = 0;
    // virtual FGfxSamplerHandle GetOrCreateSampler(const FGfxSamplerDesc& SamplerDesc) = 0;
    //
    // virtual void DestroyBuffer(FGfxBufferHandle BufferHandle) = 0;
    // virtual void DestroyTexture(FGfxTextureHandle TextureHandle) = 0;
    // virtual void DestroyTextureView(FGfxTextureViewHandle TextureViewHandle) = 0;
    //
    // [[nodiscard]] virtual uint32 GetBindlessIndex(FGfxBufferHandle BufferHandle, EGfxViewType ViewType = EGfxViewType::ShaderResource) const = 0;
    // [[nodiscard]] virtual uint32 GetBindlessIndex(FGfxTextureHandle TextureHandle, EGfxViewType ViewType = EGfxViewType::ShaderResource) const = 0;
    // [[nodiscard]] virtual uint32 GetBindlessIndex(FGfxTextureViewHandle TextureViewHandle) const = 0;
    // [[nodiscard]] virtual uint32 GetBindlessIndex(FGfxSamplerHandle SamplerHandle) const = 0;
    //
    // [[nodiscard]] virtual TSpan<byte8> Map(FGfxBufferHandle BufferHandle) = 0;
    // [[nodiscard]] virtual FGfxTextureCopyLayout GetCopyLayout(FGfxTextureHandle TextureHandle, uint32 Subresource) const = 0;
    //
    // virtual TResult<FGfxShaderHandle> CreateShader(const FGfxShaderDesc& ShaderDesc) = 0;
    // virtual TResult<FGfxPipelineHandle> CreateGraphicsPipeline(const FGfxGraphicsPipelineDesc& GraphicsPipelineDesc, FStringView DebugName = "UnnamedGraphicsPipeline") = 0;
    // virtual TResult<FGfxPipelineHandle> CreateComputePipeline(const FGfxComputePipelineDesc& ComputePipelineDesc, FStringView DebugName = "UnnamedComputePipeline") = 0;
    //
    // virtual TResult<TUniquePtr<IGfxCommandList>> CreateSwapchain(const FGfxSwapchainDesc& SwapchainDesc) = 0;
    //
    [[nodiscard]] virtual IGfxCommandList* AcquireCommandList(EGfxQueueType QueueType) = 0;
    virtual FGfxSyncPoint Submit(const FGfxSubmitDesc& SubmitDesc) = 0;
    virtual void WaitForSyncPoint(FGfxSyncPoint SyncPoint) = 0;
    [[nodiscard]] virtual bool8 IsComplete(FGfxSyncPoint SyncPoint) const = 0;
    virtual void WaitForIdle() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    [[nodiscard]] virtual uint64 GetFrameIndex() const = 0;
};

[[nodiscard]] RAVEN_API TResult<TSharedPtr<IGfxDevice>> CreateGfxDevice(const FGfxDeviceDesc& DeviceDesc);
