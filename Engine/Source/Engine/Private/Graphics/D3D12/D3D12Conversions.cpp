// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12Conversions.hpp"

#include <algorithm>

namespace
{
    constexpr FD3D12FormatInfo MakeColorFormat(const DXGI_FORMAT Format, const bool8 bIsSrgb = false)
    {
        return FD3D12FormatInfo{
            .ResourceFormat = Format,
            .ViewFormat = Format,
            .bIsSrgb = bIsSrgb,
        };
    }

    constexpr FD3D12FormatInfo MakeDepthFormat(const DXGI_FORMAT Typeless, const DXGI_FORMAT DepthStencil, const DXGI_FORMAT DepthView, const DXGI_FORMAT StencilView = DXGI_FORMAT_UNKNOWN)
    {
        return FD3D12FormatInfo{
            .ResourceFormat = Typeless,
            .ViewFormat = DepthView,
            .DepthStencilFormat = DepthStencil,
            .StencilViewFormat = StencilView,
            .bIsDepth = true,
            .bHasStencil = StencilView != DXGI_FORMAT_UNKNOWN,
        };
    }

    D3D12_FILTER_TYPE ToD3D12FilterType(const EGfxFilter Filter)
    {
        return Filter == EGfxFilter::Point ? D3D12_FILTER_TYPE_POINT : D3D12_FILTER_TYPE_LINEAR;
    }

    D3D12_FILTER_REDUCTION_TYPE ToD3D12FilterReduction(const EGfxSamplerReduction Reduction)
    {
        switch (Reduction)
        {
        case EGfxSamplerReduction::Standard: return D3D12_FILTER_REDUCTION_TYPE_STANDARD;
        case EGfxSamplerReduction::Comparison: return D3D12_FILTER_REDUCTION_TYPE_COMPARISON;
        case EGfxSamplerReduction::Minimum: return D3D12_FILTER_REDUCTION_TYPE_MINIMUM;
        case EGfxSamplerReduction::Maximum: return D3D12_FILTER_REDUCTION_TYPE_MAXIMUM;
        default: return D3D12_FILTER_REDUCTION_TYPE_STANDARD;
        }
    }

    D3D12_TEXTURE_ADDRESS_MODE ToD3D12AddressMode(const EGfxAddressMode AddressMode)
    {
        switch (AddressMode)
        {
        case EGfxAddressMode::Wrap: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case EGfxAddressMode::Mirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case EGfxAddressMode::Clamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case EGfxAddressMode::Border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case EGfxAddressMode::MirrorOnce: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
    }

    D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(const EGfxCompareOp CompareOp)
    {
        switch (CompareOp)
        {
        case EGfxCompareOp::Never: return D3D12_COMPARISON_FUNC_NEVER;
        case EGfxCompareOp::Less: return D3D12_COMPARISON_FUNC_LESS;
        case EGfxCompareOp::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
        case EGfxCompareOp::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case EGfxCompareOp::Greater: return D3D12_COMPARISON_FUNC_GREATER;
        case EGfxCompareOp::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case EGfxCompareOp::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case EGfxCompareOp::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
        default: return D3D12_COMPARISON_FUNC_NEVER;
        }
    }

    template <typename TNative>
    TNative OrFlags(const TNative A, const TNative B)
    {
        return static_cast<TNative>(static_cast<uint32>(A) | static_cast<uint32>(B));
    }
}

FD3D12FormatInfo GetD3D12FormatInfo(const EGfxFormat Format)
{
    switch (Format)
    {
    case EGfxFormat::R8_UNORM: return MakeColorFormat(DXGI_FORMAT_R8_UNORM);
    case EGfxFormat::R8_UINT: return MakeColorFormat(DXGI_FORMAT_R8_UINT);
    case EGfxFormat::RG8_UNORM: return MakeColorFormat(DXGI_FORMAT_R8G8_UNORM);
    case EGfxFormat::RGBA8_UNORM: return MakeColorFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
    case EGfxFormat::RGBA8_SRGB: return MakeColorFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, true);
    case EGfxFormat::BGRA8_UNORM: return MakeColorFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
    case EGfxFormat::BGRA8_SRGB: return MakeColorFormat(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, true);

    case EGfxFormat::R16_UNORM: return MakeColorFormat(DXGI_FORMAT_R16_UNORM);
    case EGfxFormat::R16_UINT: return MakeColorFormat(DXGI_FORMAT_R16_UINT);
    case EGfxFormat::R16_FLOAT: return MakeColorFormat(DXGI_FORMAT_R16_FLOAT);
    case EGfxFormat::RG16_FLOAT: return MakeColorFormat(DXGI_FORMAT_R16G16_FLOAT);
    case EGfxFormat::RGBA16_UNORM: return MakeColorFormat(DXGI_FORMAT_R16G16B16A16_UNORM);
    case EGfxFormat::RGBA16_FLOAT: return MakeColorFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);

    case EGfxFormat::R32_UINT: return MakeColorFormat(DXGI_FORMAT_R32_UINT);
    case EGfxFormat::R32_FLOAT: return MakeColorFormat(DXGI_FORMAT_R32_FLOAT);
    case EGfxFormat::RG32_UINT: return MakeColorFormat(DXGI_FORMAT_R32G32_UINT);
    case EGfxFormat::RG32_FLOAT: return MakeColorFormat(DXGI_FORMAT_R32G32_FLOAT);
    case EGfxFormat::RGBA32_UINT: return MakeColorFormat(DXGI_FORMAT_R32G32B32A32_UINT);
    case EGfxFormat::RGBA32_FLOAT: return MakeColorFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);

    case EGfxFormat::RGB10A2_UNORM: return MakeColorFormat(DXGI_FORMAT_R10G10B10A2_UNORM);
    case EGfxFormat::RG11B10_FLOAT: return MakeColorFormat(DXGI_FORMAT_R11G11B10_FLOAT);
    case EGfxFormat::RGB9E5_FLOAT: return MakeColorFormat(DXGI_FORMAT_R9G9B9E5_SHAREDEXP);

    case EGfxFormat::D16_UNORM: return MakeDepthFormat(DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_R16_UNORM);
    case EGfxFormat::D32_FLOAT: return MakeDepthFormat(DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
    case EGfxFormat::D24_UNORM_S8_UINT: return MakeDepthFormat(DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_X24_TYPELESS_G8_UINT);
    case EGfxFormat::D32_FLOAT_S8_UINT: return MakeDepthFormat(DXGI_FORMAT_R32G8X24_TYPELESS, DXGI_FORMAT_D32_FLOAT_S8X24_UINT, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
                                                               DXGI_FORMAT_X32_TYPELESS_G8X24_UINT);

    case EGfxFormat::BC1_UNORM: return MakeColorFormat(DXGI_FORMAT_BC1_UNORM);
    case EGfxFormat::BC1_SRGB: return MakeColorFormat(DXGI_FORMAT_BC1_UNORM_SRGB, true);
    case EGfxFormat::BC3_UNORM: return MakeColorFormat(DXGI_FORMAT_BC3_UNORM);
    case EGfxFormat::BC3_SRGB: return MakeColorFormat(DXGI_FORMAT_BC3_UNORM_SRGB, true);
    case EGfxFormat::BC4_UNORM: return MakeColorFormat(DXGI_FORMAT_BC4_UNORM);
    case EGfxFormat::BC5_UNORM: return MakeColorFormat(DXGI_FORMAT_BC5_UNORM);
    case EGfxFormat::BC6H_UFLOAT: return MakeColorFormat(DXGI_FORMAT_BC6H_UF16);
    case EGfxFormat::BC7_UNORM: return MakeColorFormat(DXGI_FORMAT_BC7_UNORM);
    case EGfxFormat::BC7_SRGB: return MakeColorFormat(DXGI_FORMAT_BC7_UNORM_SRGB, true);

    default:
        return {};
    }
}

D3D12_HEAP_TYPE ToD3D12HeapType(const EGfxMemoryType MemoryType)
{
    switch (MemoryType)
    {
    case EGfxMemoryType::GpuOnly: return D3D12_HEAP_TYPE_DEFAULT;
    case EGfxMemoryType::Upload: return D3D12_HEAP_TYPE_UPLOAD;
    case EGfxMemoryType::Readback: return D3D12_HEAP_TYPE_READBACK;
    case EGfxMemoryType::GpuUpload: return D3D12_HEAP_TYPE_GPU_UPLOAD;
    default:
        return D3D12_HEAP_TYPE_DEFAULT;
    }
}

D3D12_RESOURCE_DIMENSION ToD3D12ResourceDimension(const EGfxTextureDimension Dimension)
{
    switch (Dimension)
    {
    case EGfxTextureDimension::Texture1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    case EGfxTextureDimension::Texture2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    case EGfxTextureDimension::Texture3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    case EGfxTextureDimension::TextureCube: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    default:
        return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    }
}

D3D12_BARRIER_SYNC ToD3D12BarrierSync(const EGfxBarrierSync Sync)
{
    if (Sync == EGfxBarrierSync::None)
    {
        return D3D12_BARRIER_SYNC_NONE;
    }
    if (HasAnyFlags(Sync, EGfxBarrierSync::All))
    {
        return D3D12_BARRIER_SYNC_ALL;
    }

    struct FEntry
    {
        EGfxBarrierSync Sync;
        D3D12_BARRIER_SYNC Native;
    };
    static constexpr FEntry Table[] = {
        {EGfxBarrierSync::Draw, D3D12_BARRIER_SYNC_DRAW},
        {EGfxBarrierSync::IndexInput, D3D12_BARRIER_SYNC_INDEX_INPUT},
        {EGfxBarrierSync::VertexShading, D3D12_BARRIER_SYNC_VERTEX_SHADING},
        {EGfxBarrierSync::PixelShading, D3D12_BARRIER_SYNC_PIXEL_SHADING},
        {EGfxBarrierSync::DepthStencil, D3D12_BARRIER_SYNC_DEPTH_STENCIL},
        {EGfxBarrierSync::RenderTarget, D3D12_BARRIER_SYNC_RENDER_TARGET},
        {EGfxBarrierSync::ComputeShading, D3D12_BARRIER_SYNC_COMPUTE_SHADING},
        {EGfxBarrierSync::AllShading, D3D12_BARRIER_SYNC_ALL_SHADING},
        {EGfxBarrierSync::Copy, D3D12_BARRIER_SYNC_COPY},
        {EGfxBarrierSync::Resolve, D3D12_BARRIER_SYNC_RESOLVE},
        {EGfxBarrierSync::ExecuteIndirect, D3D12_BARRIER_SYNC_EXECUTE_INDIRECT},
        {EGfxBarrierSync::BuildAccelerationStructure, D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE},
        {EGfxBarrierSync::Raytracing, D3D12_BARRIER_SYNC_RAYTRACING},
    };

    D3D12_BARRIER_SYNC Result = D3D12_BARRIER_SYNC_NONE;
    for (const FEntry& Entry : Table)
    {
        if (HasAnyFlags(Sync, Entry.Sync))
        {
            Result = OrFlags(Result, Entry.Native);
        }
    }
    return Result;
}

D3D12_BARRIER_ACCESS ToD3D12BarrierAccess(const EGfxBarrierAccess Access)
{
    if (HasAnyFlags(Access, EGfxBarrierAccess::NoAccess))
    {
        return D3D12_BARRIER_ACCESS_NO_ACCESS;
    }

    struct FEntry
    {
        EGfxBarrierAccess Access;
        D3D12_BARRIER_ACCESS Native;
    };
    static constexpr FEntry Table[] = {
        {EGfxBarrierAccess::VertexBuffer, D3D12_BARRIER_ACCESS_VERTEX_BUFFER},
        {EGfxBarrierAccess::ConstantBuffer, D3D12_BARRIER_ACCESS_CONSTANT_BUFFER},
        {EGfxBarrierAccess::IndexBuffer, D3D12_BARRIER_ACCESS_INDEX_BUFFER},
        {EGfxBarrierAccess::RenderTarget, D3D12_BARRIER_ACCESS_RENDER_TARGET},
        {EGfxBarrierAccess::UnorderedAccess, D3D12_BARRIER_ACCESS_UNORDERED_ACCESS},
        {EGfxBarrierAccess::DepthStencilWrite, D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE},
        {EGfxBarrierAccess::DepthStencilRead, D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ},
        {EGfxBarrierAccess::ShaderResource, D3D12_BARRIER_ACCESS_SHADER_RESOURCE},
        {EGfxBarrierAccess::IndirectArgument, D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT},
        {EGfxBarrierAccess::CopySource, D3D12_BARRIER_ACCESS_COPY_SOURCE},
        {EGfxBarrierAccess::CopyDestination, D3D12_BARRIER_ACCESS_COPY_DEST},
        {EGfxBarrierAccess::ResolveSource, D3D12_BARRIER_ACCESS_RESOLVE_SOURCE},
        {EGfxBarrierAccess::ResolveDestination, D3D12_BARRIER_ACCESS_RESOLVE_DEST},
        {EGfxBarrierAccess::AccelerationStructureRead, D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ},
        {EGfxBarrierAccess::AccelerationStructureWrite, D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE},
    };

    D3D12_BARRIER_ACCESS Result = D3D12_BARRIER_ACCESS_COMMON;
    for (const FEntry& Entry : Table)
    {
        if (HasAnyFlags(Access, Entry.Access))
        {
            Result = OrFlags(Result, Entry.Native);
        }
    }
    return Result;
}

D3D12_BARRIER_LAYOUT ToD3D12BarrierLayout(const EGfxTextureLayout Layout)
{
    switch (Layout)
    {
    case EGfxTextureLayout::Undefined: return D3D12_BARRIER_LAYOUT_UNDEFINED;
    case EGfxTextureLayout::Common: return D3D12_BARRIER_LAYOUT_COMMON;
    case EGfxTextureLayout::Present: return D3D12_BARRIER_LAYOUT_PRESENT;
    case EGfxTextureLayout::RenderTarget: return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
    case EGfxTextureLayout::DepthStencilWrite: return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
    case EGfxTextureLayout::DepthStencilRead: return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
    case EGfxTextureLayout::ShaderResource: return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
    case EGfxTextureLayout::UnorderedAccess: return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
    case EGfxTextureLayout::CopySource: return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
    case EGfxTextureLayout::CopyDestination: return D3D12_BARRIER_LAYOUT_COPY_DEST;
    case EGfxTextureLayout::ResolveSource: return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
    case EGfxTextureLayout::ResolveDestination: return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
    default:
        return D3D12_BARRIER_LAYOUT_UNDEFINED;
    }
}

D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE ToD3D12BeginningAccess(const EGfxLoadOp LoadOp)
{
    switch (LoadOp)
    {
    case EGfxLoadOp::Load: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
    case EGfxLoadOp::Clear: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
    case EGfxLoadOp::Discard: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
    default:
        return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
    }
}

D3D12_RENDER_PASS_ENDING_ACCESS_TYPE ToD3D12EndingAccess(const EGfxStoreOp StoreOp)
{
    return StoreOp == EGfxStoreOp::Discard ? D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD : D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
}

D3D12_SAMPLER_DESC ToD3D12SamplerDesc(const FGfxSamplerDesc& SamplerDesc)
{
    const D3D12_FILTER_REDUCTION_TYPE Reduction = ToD3D12FilterReduction(SamplerDesc.Reduction);
    const uint32 MaxAnisotropy = std::clamp<uint32>(SamplerDesc.MaxAnisotropy, 1, D3D12_MAX_MAXANISOTROPY);

    D3D12_SAMPLER_DESC NativeDesc = {};
    NativeDesc.Filter = MaxAnisotropy > 1
                            ? D3D12_ENCODE_ANISOTROPIC_FILTER(Reduction)
                            : D3D12_ENCODE_BASIC_FILTER(ToD3D12FilterType(SamplerDesc.MinFilter), ToD3D12FilterType(SamplerDesc.MagFilter), ToD3D12FilterType(SamplerDesc.MipFilter), Reduction);
    NativeDesc.AddressU = ToD3D12AddressMode(SamplerDesc.AddressU);
    NativeDesc.AddressV = ToD3D12AddressMode(SamplerDesc.AddressV);
    NativeDesc.AddressW = ToD3D12AddressMode(SamplerDesc.AddressW);
    NativeDesc.MipLODBias = SamplerDesc.MipLodBias;
    NativeDesc.MaxAnisotropy = MaxAnisotropy;
    NativeDesc.ComparisonFunc = SamplerDesc.Reduction == EGfxSamplerReduction::Comparison ? ToD3D12ComparisonFunc(SamplerDesc.CompareOp) : D3D12_COMPARISON_FUNC_NEVER;

    const float32 Alpha = SamplerDesc.BorderColor == EGfxBorderColor::TransparentBlack ? 0.0f : 1.0f;
    const float32 Color = SamplerDesc.BorderColor == EGfxBorderColor::OpaqueWhite ? 1.0f : 0.0f;
    NativeDesc.BorderColor[0] = Color;
    NativeDesc.BorderColor[1] = Color;
    NativeDesc.BorderColor[2] = Color;
    NativeDesc.BorderColor[3] = Alpha;

    NativeDesc.MinLOD = SamplerDesc.MinLod;
    NativeDesc.MaxLOD = SamplerDesc.MaxLod;
    return NativeDesc;
}

uint32 GetTextureArraySize(const FGfxTextureDesc& Desc)
{
    return Desc.Dimension == EGfxTextureDimension::Texture3D ? 1 : Desc.DepthOrArraySize;
}

TResult<FGfxSubresourceRange> ResolveSubresourceRange(const FGfxTextureDesc& Desc, const FGfxSubresourceRange& Range)
{
    const uint32 ArraySize = GetTextureArraySize(Desc);
    if (Range.BaseMip >= Desc.MipLevels || Range.BaseArraySlice >= ArraySize)
    {
        return RESULT_ERROR("Subresource range starts outside the texture");
    }

    FGfxSubresourceRange Resolved = Range;
    if (Resolved.MipCount == GfxRemainingMips)
    {
        Resolved.MipCount = Desc.MipLevels - Range.BaseMip;
    }
    if (Resolved.ArraySliceCount == GfxRemainingArraySlices)
    {
        Resolved.ArraySliceCount = ArraySize - Range.BaseArraySlice;
    }
    if (Resolved.MipCount == 0 || Resolved.BaseMip + Resolved.MipCount > Desc.MipLevels || Resolved.ArraySliceCount == 0 || Resolved.BaseArraySlice + Resolved.ArraySliceCount > ArraySize)
    {
        return RESULT_ERROR("Subresource range exceeds the texture");
    }
    return RESULT_OK(Resolved);
}

D3D12_BARRIER_SUBRESOURCE_RANGE ToD3D12BarrierRange(const FGfxTextureDesc& Desc, const FGfxSubresourceRange& ResolvedRange)
{
    const FD3D12FormatInfo FormatInfo = GetD3D12FormatInfo(Desc.Format);
    const uint32 PlaneCount = FormatInfo.bHasStencil ? 2 : 1;

    uint32 FirstPlane = 0;
    uint32 NumPlanes = PlaneCount;
    if (ResolvedRange.Aspect == EGfxTextureAspect::Depth || ResolvedRange.Aspect == EGfxTextureAspect::Color)
    {
        NumPlanes = 1;
    }
    else if (ResolvedRange.Aspect == EGfxTextureAspect::Stencil)
    {
        FirstPlane = 1;
        NumPlanes = 1;
    }

    return D3D12_BARRIER_SUBRESOURCE_RANGE{
        .IndexOrFirstMipLevel = ResolvedRange.BaseMip,
        .NumMipLevels = ResolvedRange.MipCount,
        .FirstArraySlice = ResolvedRange.BaseArraySlice,
        .NumArraySlices = ResolvedRange.ArraySliceCount,
        .FirstPlane = FirstPlane,
        .NumPlanes = NumPlanes,
    };
}

void SetD3D12DebugName(ID3D12Object* Object, const FStringView Name)
{
    if (Object == nullptr || Name.empty())
    {
        return;
    }
    TWideChar Buffer[256];
    const int32 SourceLength = static_cast<int32>(std::min<size64>(Name.size(), 255));
    const int32 Length = MultiByteToWideChar(CP_UTF8, 0, Name.data(), SourceLength, Buffer, 255);
    Buffer[Length > 0 ? Length : 0] = L'\0';
    Object->SetName(Buffer);
}
