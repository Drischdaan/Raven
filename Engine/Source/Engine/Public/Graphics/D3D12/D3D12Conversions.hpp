// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "D3D12Include.hpp"
#include "Core/CoreTypes.hpp"
#include "Core/Utility/Result.hpp"
#include "Graphics/GfxBarriers.hpp"
#include "Graphics/GfxCommandList.hpp"
#include "Graphics/GfxTypes.hpp"

struct FD3D12FormatInfo
{
    DXGI_FORMAT ResourceFormat = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT ViewFormat = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT StencilViewFormat = DXGI_FORMAT_UNKNOWN;
    bool8 bIsDepth = false;
    bool8 bHasStencil = false;
    bool8 bIsSrgb = false;
};

[[nodiscard]] FD3D12FormatInfo GetD3D12FormatInfo(EGfxFormat Format);

[[nodiscard]] D3D12_HEAP_TYPE ToD3D12HeapType(EGfxMemoryType MemoryType);
[[nodiscard]] D3D12_RESOURCE_DIMENSION ToD3D12ResourceDimension(EGfxTextureDimension Dimension);

[[nodiscard]] D3D12_BARRIER_SYNC ToD3D12BarrierSync(EGfxBarrierSync Sync);
[[nodiscard]] D3D12_BARRIER_ACCESS ToD3D12BarrierAccess(EGfxBarrierAccess Access);
[[nodiscard]] D3D12_BARRIER_LAYOUT ToD3D12BarrierLayout(EGfxTextureLayout Layout);

[[nodiscard]] D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE ToD3D12BeginningAccess(EGfxLoadOp LoadOp);
[[nodiscard]] D3D12_RENDER_PASS_ENDING_ACCESS_TYPE ToD3D12EndingAccess(EGfxStoreOp StoreOp);

[[nodiscard]] D3D12_SAMPLER_DESC ToD3D12SamplerDesc(const FGfxSamplerDesc& SamplerDesc);

[[nodiscard]] uint32 GetTextureArraySize(const FGfxTextureDesc& Desc);

[[nodiscard]] TResult<FGfxSubresourceRange> ResolveSubresourceRange(const FGfxTextureDesc& Desc, const FGfxSubresourceRange& Range);

[[nodiscard]] D3D12_BARRIER_SUBRESOURCE_RANGE ToD3D12BarrierRange(const FGfxTextureDesc& Desc, const FGfxSubresourceRange& ResolvedRange);

[[nodiscard]] constexpr uint64 AlignUp(const uint64 Value, const uint64 Alignment)
{
    return (Value + Alignment - 1) & ~(Alignment - 1);
}

[[nodiscard]] constexpr bool8 IsPowerOfTwo(const uint64 Value)
{
    return Value != 0 && (Value & (Value - 1)) == 0;
}

void SetD3D12DebugName(ID3D12Object* Object, FStringView Name);
