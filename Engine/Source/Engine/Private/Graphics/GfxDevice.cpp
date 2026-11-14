// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/GfxDevice.hpp"

#include "Graphics/D3D12/D3D12GfxDevice.hpp"

TResult<TSharedPtr<IGfxDevice>> CreateGfxDevice(const FGfxDeviceDesc& DeviceDesc)
{
    switch (DeviceDesc.Backend)
    {
    case EGfxBackend::D3D12:
        {
            TSharedPtr<FD3D12GfxDevice> Device = MakeShared<FD3D12GfxDevice>();
            RESULT_CHECK(Device->Initialize(DeviceDesc))
            return RESULT_OK(Device);
        }
    case EGfxBackend::Null:
    case EGfxBackend::Vulkan:
        break;
    }
    return RESULT_ERROR("Unable to create graphics device");
}
