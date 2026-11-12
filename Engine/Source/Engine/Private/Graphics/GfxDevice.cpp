// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/GfxDevice.hpp"

#include "Graphics/D3D12/D3D12GfxDevice.hpp"

TResult<TSharedPtr<IGfxDevice>> CreateGfxDevice(const FGfxDeviceDesc& DeviceDesc)
{
    switch (DeviceDesc.Backend)
    {
    case EGfxBackend::D3D12:
        {
            TSharedPtr<IGfxDevice> Device = MakeShared<FD3D12GfxDevice>();
            FResult Result = Device->Initialize(DeviceDesc);
            if (Result.HasError())
            {
                return std::unexpected(std::move(Result).error());
            }
            return RESULT_OK(Device);
        }
    case EGfxBackend::Null:
    case EGfxBackend::Vulkan:
        break;
    }
    return RESULT_ERROR("Unable to create graphics device");
}
