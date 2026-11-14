// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "D3D12Include.hpp"
#include "Graphics/GfxTypes.hpp"

enum class ED3D12DescriptorHeapType : uint8
{
    Resource,
    Sampler,
    RenderTarget,
    DepthStencil,
    Count,
};

enum class ED3D12RootParameter : uint8
{
    PushConstants,
    FrameConstants,
    ViewConstants,
    PassConstants,
    Count,
};

inline constexpr uint32 D3D12ResourceHeapCapacity = 262'144;
inline constexpr uint32 D3D12SamplerHeapCapacity = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
inline constexpr uint32 D3D12RenderTargetHeapCapacity = 4'096;
inline constexpr uint32 D3D12DepthStencilHeapCapacity = 1'024;

inline constexpr uint32 D3D12NullResourceIndex = 0;
inline constexpr uint32 D3D12DefaultSamplerIndex = 0;

[[nodiscard]] constexpr ED3D12RootParameter ToD3D12RootParameter(const EGfxConstantSlot Slot)
{
    switch (Slot)
    {
    case EGfxConstantSlot::Frame: return ED3D12RootParameter::FrameConstants;
    case EGfxConstantSlot::View: return ED3D12RootParameter::ViewConstants;
    case EGfxConstantSlot::Pass:
    case EGfxConstantSlot::Count:
    default:
        return ED3D12RootParameter::PassConstants;
    }
}
