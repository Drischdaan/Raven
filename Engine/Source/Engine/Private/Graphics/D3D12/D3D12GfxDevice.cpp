// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12GfxDevice.hpp"

#include <print>

#include "Core/Utility/Macros.hpp"

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
    bIsInitialized = true;
    DeviceDesc = InDeviceDesc;

    RETURN_IF_FAILED(AcquireFactory())
    RETURN_IF_FAILED(FindSuitableAdapter())
    RETURN_IF_FAILED(CreateDevice())

    return RESULT_OK(FResult());
}

void FD3D12GfxDevice::Deinitialize()
{
    if (!bIsInitialized)
    {
        return;
    }
    bIsInitialized = false;
    Adapter.Reset();
    Factory.Reset();
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
            return RESULT_ERROR("Failed to get adapter description");
        }

        if ((AdapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) == DXGI_ADAPTER_FLAG3_SOFTWARE)
        {
            continue;
        }
        if ((AdapterDesc.Flags & DXGI_ADAPTER_FLAG3_REMOTE) == DXGI_ADAPTER_FLAG3_REMOTE)
        {
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
    const HRESULT NativeResult = D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&Device));
    if (FAILED(NativeResult))
    {
        return RESULT_PLATFORM_ERROR("Failed to create D3D12 device");
    }
    return RESULT_OK(FResult());
}
