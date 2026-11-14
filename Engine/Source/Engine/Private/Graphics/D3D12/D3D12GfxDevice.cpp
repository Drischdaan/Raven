// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12GfxDevice.hpp"

#include <print>

#include "Core/Utility/Macros.hpp"
#include "Graphics/D3D12/D3D12GfxCommandList.hpp"

namespace
{
    constexpr uint32 MaxCommandListsPerSubmit = 32;

    FString WideToUtf8(const wchar_t* WideString)
    {
        if (!WideString || !*WideString)
        {
            return "";
        }
        const int size_needed = WideCharToMultiByte(CP_UTF8, 0, WideString, -1, nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0)
        {
            return "";
        }
        FString String(size_needed - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, WideString, -1, String.data(), size_needed, nullptr, nullptr);
        return String;
    }

    FString FeatureLevelToString(const D3D_FEATURE_LEVEL FeatureLevel)
    {
        switch (FeatureLevel)
        {
        case D3D_FEATURE_LEVEL_1_0_GENERIC: return "D3D_FEATURE_LEVEL_1_0_GENERIC";
        case D3D_FEATURE_LEVEL_1_0_CORE: return "D3D_FEATURE_LEVEL_1_0_CORE";
        case D3D_FEATURE_LEVEL_9_1: return "D3D_FEATURE_LEVEL_9_1";
        case D3D_FEATURE_LEVEL_9_2: return "D3D_FEATURE_LEVEL_9_2";
        case D3D_FEATURE_LEVEL_9_3: return "D3D_FEATURE_LEVEL_9_3";
        case D3D_FEATURE_LEVEL_10_0: return "D3D_FEATURE_LEVEL_10_0";
        case D3D_FEATURE_LEVEL_10_1: return "D3D_FEATURE_LEVEL_10_1";
        case D3D_FEATURE_LEVEL_11_0: return "D3D_FEATURE_LEVEL_11_0";
        case D3D_FEATURE_LEVEL_11_1: return "D3D_FEATURE_LEVEL_11_1";
        case D3D_FEATURE_LEVEL_12_0: return "D3D_FEATURE_LEVEL_12_0";
        case D3D_FEATURE_LEVEL_12_1: return "D3D_FEATURE_LEVEL_12_1";
        case D3D_FEATURE_LEVEL_12_2: return "D3D_FEATURE_LEVEL_12_2";
        default:
            return "Unknown";
        }
    }
}

FResult FD3D12GfxDevice::Initialize(const FGfxDeviceDesc& InDeviceDesc)
{
    if (bIsInitialized)
    {
        return RESULT_ERROR("D3D12 Graphics Device is already initialized");
    }
    DeviceDesc = InDeviceDesc;

    RESULT_CHECK(AcquireFactory())
    RESULT_CHECK(FindSuitableAdapter())
    RESULT_CHECK(CreateDevice())
    RESULT_CHECK(CheckFeatureSupport())
    RESULT_CHECK(CreateQueues())
    RESULT_CHECK(CreateDescriptorHeaps())
    RESULT_CHECK(CreateGlobalRootSignature())

    DeletionQueue.Initialize(Queues);
    FrameIndex = 0;
    bIsInFrame = false;
    FrameFenceValues = {};

    CreateCommandListPools();

    bIsInitialized = true;
    return RESULT_OK_EMPTY();
}

void FD3D12GfxDevice::Deinitialize()
{
    if (!bIsInitialized)
    {
        return;
    }
    WaitForIdle();
    DeletionQueue.Flush();
    ReportAndReleaseLeakedResources();

    for (FD3D12CommandListPool& CommandListPool : CommandListPools)
    {
        CommandListPool.Deinitialize();
    }
    for (FD3D12GfxQueue& Queue : Queues)
    {
        Queue.Deinitialize();
    }
    GlobalRootSignature.Reset();
    for (FD3D12DescriptorHeap& DescriptorHeap : DescriptorHeaps)
    {
        DescriptorHeap.Deinitialize();
    }
    Device.Reset();
    Adapter.Reset();
    Factory.Reset();
    bIsInitialized = false;
}

FD3D12GfxQueue& FD3D12GfxDevice::GetQueue(EGfxQueueType Type) const
{
    return const_cast<FD3D12GfxQueue&>(Queues[static_cast<uint8>(Type)]);
}

void FD3D12GfxDevice::DestroyBuffer(const FGfxBufferHandle BufferHandle)
{
    TResult<FD3D12Buffer> Removed = Buffers.Remove(BufferHandle);
    if (Removed.HasError())
    {
        std::println("DestroyBuffer: {}", Removed.GetError().Message);
        return;
    }
    FD3D12Buffer Buffer = std::move(Removed).GetValue();
    DeferRelease(std::move(Buffer.Resource));
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Buffer.SrvIndex);
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Buffer.UavIndex);
}

void FD3D12GfxDevice::DestroyTexture(const FGfxTextureHandle TextureHandle)
{
    TResult<FD3D12Texture> Removed = Textures.Remove(TextureHandle);
    if (Removed.HasError())
    {
        std::println("DestroyTexture: {}", Removed.GetError().Message);
        return;
    }
    FD3D12Texture Texture = std::move(Removed).GetValue();
    if (!Texture.bIsSwapchainImage)
    {
        DeferRelease(std::move(Texture.Resource));
    }
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Texture.SrvIndex);
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Texture.UavIndex);
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::RenderTarget, Texture.RtvIndex);
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::DepthStencil, Texture.DsvIndex);
}

void FD3D12GfxDevice::DestroyTextureView(const FGfxTextureViewHandle TextureViewHandle)
{
    TResult<FD3D12TextureView> Removed = TextureViews.Remove(TextureViewHandle);
    if (Removed.HasError())
    {
        std::println("DestroyTextureView: {}", Removed.GetError().Message);
        return;
    }
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Removed.GetValue().DescriptorIndex);
}

uint32 FD3D12GfxDevice::GetBindlessIndex(const FGfxBufferHandle BufferHandle, const EGfxViewType ViewType) const
{
    const FD3D12Buffer* Buffer = Buffers.Get(BufferHandle);
    if (Buffer == nullptr)
    {
        return GfxInvalidBindlessIndex;
    }
    return ViewType == EGfxViewType::UnorderedAccess ? Buffer->UavIndex : Buffer->SrvIndex;
}

uint32 FD3D12GfxDevice::GetBindlessIndex(const FGfxTextureHandle TextureHandle, const EGfxViewType ViewType) const
{
    const FD3D12Texture* Texture = Textures.Get(TextureHandle);
    if (Texture == nullptr)
    {
        return GfxInvalidBindlessIndex;
    }
    return ViewType == EGfxViewType::UnorderedAccess ? Texture->UavIndex : Texture->SrvIndex;
}

uint32 FD3D12GfxDevice::GetBindlessIndex(const FGfxTextureViewHandle TextureViewHandle) const
{
    const FD3D12TextureView* TextureView = TextureViews.Get(TextureViewHandle);
    return TextureView != nullptr ? TextureView->DescriptorIndex : GfxInvalidBindlessIndex;
}

uint32 FD3D12GfxDevice::GetBindlessIndex(const FGfxSamplerHandle SamplerHandle) const
{
    const FD3D12Sampler* Sampler = Samplers.Get(SamplerHandle);
    return Sampler != nullptr ? Sampler->DescriptorIndex : GfxInvalidBindlessIndex;
}

FD3D12Buffer* FD3D12GfxDevice::GetBuffer(const FGfxBufferHandle BufferHandle)
{
    return Buffers.Get(BufferHandle);
}

FD3D12Texture* FD3D12GfxDevice::GetTexture(const FGfxTextureHandle TextureHandle)
{
    return Textures.Get(TextureHandle);
}

FD3D12TextureView* FD3D12GfxDevice::GetTextureView(const FGfxTextureViewHandle TextureViewHandle)
{
    return TextureViews.Get(TextureViewHandle);
}

FD3D12Pipeline* FD3D12GfxDevice::GetPipeline(const FGfxPipelineHandle PipelineHandle)
{
    return Pipelines.Get(PipelineHandle);
}

FD3D12DescriptorHeap& FD3D12GfxDevice::GetDescriptorHeap(ED3D12DescriptorHeapType HeapType)
{
    return DescriptorHeaps[static_cast<uint8>(HeapType)];
}

ID3D12RootSignature* FD3D12GfxDevice::GetGlobalRootSignature() const
{
    return GlobalRootSignature.Get();
}

void FD3D12GfxDevice::FreeDescriptorDeferred(ED3D12DescriptorHeapType HeapType, uint32 Index)
{
    if (Index == GfxInvalidBindlessIndex)
    {
        return;
    }
    DeferRelease(nullptr, [this, HeapType, Index]
    {
        GetDescriptorHeap(HeapType).Free(Index);
    });
}

IGfxCommandList* FD3D12GfxDevice::AcquireCommandList(EGfxQueueType QueueType)
{
    return CommandListPools[static_cast<uint8>(QueueType)].Acquire();
}

FGfxSyncPoint FD3D12GfxDevice::Submit(const FGfxSubmitDesc& SubmitDesc)
{
    if (SubmitDesc.CommandLists.size() > MaxCommandListsPerSubmit)
    {
        std::println("Submit: too many command lists ({} > {})", SubmitDesc.CommandLists.size(), MaxCommandListsPerSubmit);
        return {};
    }

    TArray<FD3D12GfxCommandList*, MaxCommandListsPerSubmit> D3D12Lists;
    TArray<ID3D12CommandList*, MaxCommandListsPerSubmit> NativeLists;
    uint32 Count = 0;

    for (IGfxCommandList* CommandList : SubmitDesc.CommandLists)
    {
        FD3D12GfxCommandList* D3D12List = dynamic_cast<FD3D12GfxCommandList*>(CommandList);
        if (D3D12List->GetQueueType() != SubmitDesc.QueueType)
        {
            std::println("Submit: command list was acquired for a different queue type");
            return {};
        }
        if (!D3D12List->IsRecording())
        {
            std::println("Submit: command list was already submitted");
            return {};
        }
        D3D12List->Close();
        D3D12Lists[Count] = D3D12List;
        NativeLists[Count] = D3D12List->GetNative();
        ++Count;
    }

    const FGfxSyncPoint SyncPoint = GetQueue(SubmitDesc.QueueType).Submit({NativeLists.data(), Count}, SubmitDesc.WaitFor, *this);
    CommandListPools[static_cast<uint8>(SubmitDesc.QueueType)].Recycle({D3D12Lists.data(), Count}, SyncPoint.Value);
    return SyncPoint;
}

void FD3D12GfxDevice::WaitForSyncPoint(const FGfxSyncPoint SyncPoint)
{
    GetQueue(SyncPoint.QueueType).CpuWait(SyncPoint.Value);
}

bool8 FD3D12GfxDevice::IsComplete(const FGfxSyncPoint SyncPoint) const
{
    return GetQueue(SyncPoint.QueueType).IsComplete(SyncPoint.Value);
}

void FD3D12GfxDevice::WaitForIdle()
{
    for (FD3D12GfxQueue& Queue : Queues)
    {
        Queue.WaitForIdle();
    }
}

void FD3D12GfxDevice::BeginFrame()
{
    if (bIsInFrame)
    {
        std::println("BeginFrame called twice without EndFrame");
        return;
    }

    const uint32 Slot = GetFrameSlot();
    WaitForFenceValues(FrameFenceValues[Slot]);
    DeletionQueue.Process();

    bIsInFrame = true;
}

void FD3D12GfxDevice::EndFrame()
{
    if (!bIsInFrame)
    {
        std::println("EndFrame called without BeginFrame");
        return;
    }

    GetQueue(EGfxQueueType::Graphics).Signal();

    const FD3D12QueueFenceValues FenceValues = CaptureLastSubmittedValues();
    FrameFenceValues[GetFrameSlot()] = FenceValues;
    DeletionQueue.Retire(FenceValues);

    ++FrameIndex;
    bIsInFrame = false;
}

uint64 FD3D12GfxDevice::GetFrameIndex() const
{
    return FrameIndex;
}

uint32 FD3D12GfxDevice::GetFrameSlot() const
{
    return static_cast<uint32>(FrameIndex % DeviceDesc.FramesInFlight);
}

void FD3D12GfxDevice::DeferRelease(TComPtr<IUnknown> Object, FD3D12DeferredDeletionQueue::FReleaseCallback OnRelease)
{
    DeletionQueue.Enqueue(std::move(Object), std::move(OnRelease));
}

FResult FD3D12GfxDevice::AcquireFactory()
{
    uint32 FactoryCreateFlags = 0;
    if (DeviceDesc.bEnableDebugLayer)
    {
        FactoryCreateFlags = DXGI_CREATE_FACTORY_DEBUG;
    }
    RESULT_TRY_PLATFORM_CALL(CreateDXGIFactory2(FactoryCreateFlags, IID_PPV_ARGS(&Factory)), "Unable to create dxgi factory")
    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxDevice::FindSuitableAdapter()
{
    TComPtr<IDXGIAdapter4> TempAdapter;
    for (uint32 Index = 0; Factory->EnumAdapterByGpuPreference(Index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&TempAdapter)) != DXGI_ERROR_NOT_FOUND; ++Index)
    {
        DXGI_ADAPTER_DESC3 AdapterDesc = {};
        RESULT_TRY_PLATFORM_CALL_CLEANUP(TempAdapter->GetDesc3(&AdapterDesc), TempAdapter.Reset();, "Failed to get adapter description")
        if ((AdapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) == DXGI_ADAPTER_FLAG3_SOFTWARE)
        {
            TempAdapter.Reset();
            continue;
        }
        if ((AdapterDesc.Flags & DXGI_ADAPTER_FLAG3_REMOTE) == DXGI_ADAPTER_FLAG3_REMOTE)
        {
            TempAdapter.Reset();
            continue;
        }
        std::println("Chosen Adapter: {}", WideToUtf8(AdapterDesc.Description));
        break;
    }
    if (TempAdapter == nullptr)
    {
        return RESULT_ERROR("Unable to find suitable adapter");
    }
    Adapter = TempAdapter;
    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxDevice::CreateDevice()
{
    D3D_FEATURE_LEVEL FeatureLevels[5] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D_FEATURE_LEVEL HighestSupportedFeatureLevel = FeatureLevels[_countof(FeatureLevels) - 1];
    for (const D3D_FEATURE_LEVEL& FeatureLevelProbe : FeatureLevels)
    {
        const HRESULT NativeResult = D3D12CreateDevice(Adapter.Get(), FeatureLevelProbe, __uuidof(ID3D12Device), nullptr);
        if (FAILED(NativeResult))
        {
            continue;
        }
        HighestSupportedFeatureLevel = FeatureLevelProbe;
        break;
    }

    if (HighestSupportedFeatureLevel < D3D_FEATURE_LEVEL_12_0)
    {
        return RESULT_ERROR("Adapter must at least support feature level 12.0");
    }
    FeatureLevel = HighestSupportedFeatureLevel;
    RESULT_TRY_PLATFORM_CALL_CLEANUP(D3D12CreateDevice(Adapter.Get(), FeatureLevel, IID_PPV_ARGS(&Device)), Device.Reset();, "Failed to create D3D12 device")
    std::println("Created device with feature level {}", FeatureLevelToString(FeatureLevel));
    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxDevice::CheckFeatureSupport()
{
    BOOL bSupportsTearing = FALSE;
    HRESULT NativeResult = Factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &bSupportsTearing, sizeof(bSupportsTearing));
    if (FAILED(NativeResult))
    {
        bSupportsTearing = FALSE;
    }
    Capabilities.bSupportsTearing = bSupportsTearing;

    D3D12_FEATURE_DATA_D3D12_OPTIONS Options = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &Options, sizeof(Options)), "Failed to check feature options")
    if (Options.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3)
    {
        return RESULT_ERROR("Bindless rendering requires resource binding tier 3");
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 Options5 = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &Options5, sizeof(Options5)), "Failed to check feature options")
    Capabilities.bSupportsRayTracing = Options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;

    D3D12_FEATURE_DATA_D3D12_OPTIONS6 Options6 = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &Options6, sizeof(Options6)), "Failed to check feature options")
    Capabilities.bSupportsVariableRateShading = Options6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1;

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 Options7 = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &Options7, sizeof(Options7)), "Failed to check feature options")
    Capabilities.bSupportsMeshShaders = Options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1;

    D3D12_FEATURE_DATA_D3D12_OPTIONS16 Options16 = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &Options16, sizeof(Options16)), "Failed to check feature options")
    Capabilities.bSupportsGpuUploadHeap = Options16.GPUUploadHeapSupported;

    D3D12_FEATURE_DATA_SHADER_MODEL ShaderModel = {.HighestShaderModel = D3D_SHADER_MODEL_6_6};
    NativeResult = Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &ShaderModel, sizeof(ShaderModel));
    if (FAILED(NativeResult) || ShaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_6)
    {
        return RESULT_ERROR("Bindless rendering requires shader model 6.6 (ResourceDescriptorHeap)");
    }

    std::println("Device Capabilities:");
    std::println(" - Tearing: {}", Capabilities.bSupportsTearing);
    std::println(" - RayTracing: {}", Capabilities.bSupportsRayTracing);
    std::println(" - VariableRateShading: {}", Capabilities.bSupportsVariableRateShading);
    std::println(" - MeshShaders: {}", Capabilities.bSupportsMeshShaders);
    std::println(" - GpuUploadHeap: {}", Capabilities.bSupportsGpuUploadHeap);

    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxDevice::CreateQueues()
{
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        RESULT_CHECK(Queues[Index].Initialize(Device.Get(), static_cast<EGfxQueueType>(Index)))
    }
    return RESULT_OK_EMPTY();
}

void FD3D12GfxDevice::CreateCommandListPools()
{
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        CommandListPools[Index].Initialize(*this, Device.Get(), Queues[Index]);
    }
}

FResult FD3D12GfxDevice::CreateDescriptorHeaps()
{
    const FD3D12DescriptorHeapDesc HeapDescs[] = {
        {
            .HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .Capacity = D3D12ResourceHeapCapacity,
            .ReservedCount = 1,
            .bIsShaderVisible = true,
            .DebugName = L"BindlessResourceHeap",
        },
        {
            .HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
            .Capacity = D3D12SamplerHeapCapacity,
            .ReservedCount = 1,
            .bIsShaderVisible = true,
            .DebugName = L"BindlessSamplerHeap",
        },
        {
            .HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .Capacity = D3D12RenderTargetHeapCapacity,
            .ReservedCount = 0,
            .bIsShaderVisible = false,
            .DebugName = L"RenderTargetViewHeap",
        },
        {
            .HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .Capacity = D3D12DepthStencilHeapCapacity,
            .ReservedCount = 0,
            .bIsShaderVisible = false,
            .DebugName = L"DepthStencilViewHeap",
        },
    };
    static_assert(std::extent_v<decltype(HeapDescs)> == static_cast<size64>(ED3D12DescriptorHeapType::Count));
    for (uint8 Index = 0; Index < static_cast<uint8>(ED3D12DescriptorHeapType::Count); ++Index)
    {
        RESULT_CHECK(DescriptorHeaps[Index].Initialize(Device.Get(), HeapDescs[Index]))
    }

    WriteDefaultDescriptors();
    return RESULT_OK_EMPTY();
}

void FD3D12GfxDevice::WriteDefaultDescriptors()
{
    // Resource index 0: null Texture2D SRV. Loads and samples return 0.
    D3D12_SHADER_RESOURCE_VIEW_DESC NullSrvDesc = {};
    NullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    NullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    NullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    NullSrvDesc.Texture2D.MipLevels = 1;
    Device->CreateShaderResourceView(nullptr, &NullSrvDesc, GetDescriptorHeap(ED3D12DescriptorHeapType::Resource).GetDescriptor(D3D12NullResourceIndex).CpuHandle);

    // Sampler index 0: trilinear wrap.
    D3D12_SAMPLER_DESC DefaultSamplerDesc = {};
    DefaultSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    DefaultSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    DefaultSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    DefaultSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    DefaultSamplerDesc.MaxAnisotropy = 1;
    DefaultSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    DefaultSamplerDesc.MinLOD = 0.0f;
    DefaultSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    Device->CreateSampler(&DefaultSamplerDesc, GetDescriptorHeap(ED3D12DescriptorHeapType::Sampler).GetDescriptor(D3D12DefaultSamplerIndex).CpuHandle);
}

FResult FD3D12GfxDevice::CreateGlobalRootSignature()
{
    D3D12_ROOT_PARAMETER1 Parameters[static_cast<uint8>(ED3D12RootParameter::Count)] = {};

    D3D12_ROOT_PARAMETER1& PushConstants = Parameters[static_cast<uint8>(ED3D12RootParameter::PushConstants)];
    PushConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    PushConstants.Constants.ShaderRegister = 0;
    PushConstants.Constants.RegisterSpace = 0;
    PushConstants.Constants.Num32BitValues = GfxMaxPushConstantsSize / sizeof(uint32);
    PushConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    constexpr ED3D12RootParameter ConstantBufferParameters[] = {
        ED3D12RootParameter::FrameConstants,
        ED3D12RootParameter::ViewConstants,
        ED3D12RootParameter::PassConstants,
    };
    for (uint32 Index = 0; Index < std::size(ConstantBufferParameters); ++Index)
    {
        D3D12_ROOT_PARAMETER1& Parameter = Parameters[static_cast<uint8>(ConstantBufferParameters[Index])];
        Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        Parameter.Descriptor.ShaderRegister = 1 + Index;
        Parameter.Descriptor.RegisterSpace = 0;
        Parameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        Parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc = {};
    RootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    RootSignatureDesc.Desc_1_1.NumParameters = static_cast<uint32>(std::size(Parameters));
    RootSignatureDesc.Desc_1_1.pParameters = Parameters;
    RootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
    RootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
    RootSignatureDesc.Desc_1_1.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
        D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

    TComPtr<ID3DBlob> SignatureBlob;
    TComPtr<ID3DBlob> ErrorBlob;
    HRESULT NativeResult = D3D12SerializeVersionedRootSignature(&RootSignatureDesc, &SignatureBlob, &ErrorBlob);
    if (FAILED(NativeResult))
    {
        FString Message = "Failed to serialize global root signature";
        if (ErrorBlob)
        {
            Message += ": ";
            Message.append(static_cast<const char*>(ErrorBlob->GetBufferPointer()), ErrorBlob->GetBufferSize());
        }
        return RESULT_PLATFORM_ERROR(NativeResult, Message);
    }

    NativeResult = Device->CreateRootSignature(0, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), IID_PPV_ARGS(&GlobalRootSignature));
    if (FAILED(NativeResult))
    {
        return RESULT_PLATFORM_ERROR(NativeResult, "Failed to create global root signature");
    }
    GlobalRootSignature->SetName(L"GlobalRootSignature");
    return RESULT_OK(FResult());
}

FD3D12QueueFenceValues FD3D12GfxDevice::CaptureLastSubmittedValues() const
{
    FD3D12QueueFenceValues Result;
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        Result.Values[Index] = Queues[Index].GetLastSubmittedValue();
    }
    return Result;
}

void FD3D12GfxDevice::WaitForFenceValues(const FD3D12QueueFenceValues& FenceValues)
{
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        Queues[Index].CpuWait(FenceValues.Values[Index]);
    }
}

void FD3D12GfxDevice::ReportAndReleaseLeakedResources()
{
    const uint32 LeakedBuffers = Buffers.GetAliveCount();
    const uint32 LeakedTextures = Textures.GetAliveCount();
    const uint32 LeakedTextureViews = TextureViews.GetAliveCount();
    if (LeakedBuffers + LeakedTextures + LeakedTextureViews > 0)
    {
        std::println("Gfx device shut down with {} buffers, {} textures and {} texture views still alive", LeakedBuffers, LeakedTextures, LeakedTextureViews);
    }
    Pipelines.Clear();
    Shaders.Clear();
    Samplers.Clear();
    TextureViews.Clear();
    Textures.Clear();
    Buffers.Clear();
}
