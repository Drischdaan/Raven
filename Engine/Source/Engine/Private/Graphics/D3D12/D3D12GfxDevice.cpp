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

#define RETURN_IF_FAILED(ResultMethod) \
    FResult RAVEN_UNIQUE_VARIABLE(Result) = ResultMethod; \
    if(RAVEN_UNIQUE_VARIABLE(Result).HasError()) \
    { \
        return RAVEN_UNIQUE_VARIABLE(Result); \
    }

FResult FD3D12GfxDevice::Initialize(const FGfxDeviceDesc& InDeviceDesc)
{
    if (bIsInitialized)
    {
        return RESULT_ERROR("D3D12 Graphics Device is already initialized");
    }
    DeviceDesc = InDeviceDesc;

    RETURN_IF_FAILED(AcquireFactory())
    RETURN_IF_FAILED(FindSuitableAdapter())
    RETURN_IF_FAILED(CreateDevice())
    RETURN_IF_FAILED(CheckFeatureSupport())
    RETURN_IF_FAILED(CreateQueues())

    DeletionQueue.Initialize(Queues);
    FrameIndex = 0;
    bIsInFrame = false;
    FrameFenceValues = {};

    CreateCommandListPools();

    bIsInitialized = true;
    return RESULT_OK(FResult());
}

void FD3D12GfxDevice::Deinitialize()
{
    if (!bIsInitialized)
    {
        return;
    }
    WaitForIdle();
    DeletionQueue.Flush();

    for (FD3D12CommandListPool& CommandListPool : CommandListPools)
    {
        CommandListPool.Deinitialize();
    }
    for (FD3D12GfxQueue& Queue : Queues)
    {
        Queue.Deinitialize();
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
    const HRESULT NativeResult = CreateDXGIFactory2(FactoryCreateFlags, IID_PPV_ARGS(&Factory));
    if (FAILED(NativeResult))
    {
        return RESULT_PLATFORM_ERROR("Unable to create dxgi factory");
    }
    return RESULT_OK(FResult());
}

FResult FD3D12GfxDevice::FindSuitableAdapter()
{
    TComPtr<IDXGIAdapter4> TempAdapter;
    for (uint32 Index = 0; Factory->EnumAdapterByGpuPreference(Index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&TempAdapter)) != DXGI_ERROR_NOT_FOUND; ++Index)
    {
        DXGI_ADAPTER_DESC3 AdapterDesc = {};
        const HRESULT NativeResult = TempAdapter->GetDesc3(&AdapterDesc);
        if (FAILED(NativeResult))
        {
            TempAdapter.Reset();
            return RESULT_PLATFORM_ERROR("Failed to get adapter description");
        }

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
    return RESULT_OK(FResult());
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

    const HRESULT NativeResult = D3D12CreateDevice(Adapter.Get(), FeatureLevel, IID_PPV_ARGS(&Device));
    if (FAILED(NativeResult))
    {
        return RESULT_PLATFORM_ERROR("Failed to create D3D12 device");
    }
    std::println("Created device with feature level {}", FeatureLevelToString(FeatureLevel));
    return RESULT_OK(FResult());
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

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 Options5 = {};
    NativeResult = Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &Options5, sizeof(Options5));
    if (FAILED(NativeResult))
    {
        return RESULT_PLATFORM_ERROR("Failed to check feature options");
    }
    Capabilities.bSupportsRayTracing = Options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;

    D3D12_FEATURE_DATA_D3D12_OPTIONS6 Options6 = {};
    NativeResult = Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &Options6, sizeof(Options6));
    if (FAILED(NativeResult))
    {
        return RESULT_PLATFORM_ERROR("Failed to check feature options");
    }
    Capabilities.bSupportsVariableRateShading = Options6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1;

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 Options7 = {};
    NativeResult = Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &Options7, sizeof(Options7));
    if (FAILED(NativeResult))
    {
        return RESULT_PLATFORM_ERROR("Failed to check feature options");
    }
    Capabilities.bSupportsMeshShaders = Options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1;

    D3D12_FEATURE_DATA_D3D12_OPTIONS16 Options16 = {};
    NativeResult = Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &Options16, sizeof(Options16));
    if (FAILED(NativeResult))
    {
        return RESULT_PLATFORM_ERROR("Failed to check feature options");
    }
    Capabilities.bSupportsGpuUploadHeap = Options16.GPUUploadHeapSupported;

    std::println("Device Capabilities:");
    std::println(" - Tearing: {}", Capabilities.bSupportsTearing);
    std::println(" - RayTracing: {}", Capabilities.bSupportsRayTracing);
    std::println(" - VariableRateShading: {}", Capabilities.bSupportsVariableRateShading);
    std::println(" - MeshShaders: {}", Capabilities.bSupportsMeshShaders);
    std::println(" - GpuUploadHeap: {}", Capabilities.bSupportsGpuUploadHeap);

    return RESULT_OK(FResult());
}

FResult FD3D12GfxDevice::CreateQueues()
{
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        RETURN_IF_FAILED(Queues[Index].Initialize(Device.Get(), static_cast<EGfxQueueType>(Index)))
    }
    return RESULT_OK(FResult());
}

void FD3D12GfxDevice::CreateCommandListPools()
{
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        CommandListPools[Index].Initialize(*this, Device.Get(), Queues[Index]);
    }
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
