// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <optional>

#include "GfxTypes.hpp"
#include "Core/Memory/Containers/Array.hpp"
#include "Core/Memory/Containers/Span.hpp"

struct FGfxClearValue
{
    TArray<float32, 4> Color = {};
    float32 Depth = 0.0f;
    uint8 Stencil = 0;
};

struct FGfxBufferDesc
{
    uint64 Size = 0;
    uint32 Stride = 0;
    EGfxBufferUsage Usage = EGfxBufferUsage::None;
    EGfxMemoryType MemoryType = EGfxMemoryType::GpuOnly;
};

struct FGfxTextureDesc
{
    uint32 Width = 1;
    uint32 Height = 1;
    uint32 DepthOrArraySize = 1;
    uint32 MipLevels = 1;
    uint32 SampleCount = 1;
    EGfxFormat Format = EGfxFormat::Unknown;
    EGfxTextureDimension Dimension = EGfxTextureDimension::Texture2D;
    EGfxTextureUsage Usage = EGfxTextureUsage::ShaderResource;
    std::optional<FGfxClearValue> OptimizedClearValue;
};

struct FGfxTextureViewDesc
{
    EGfxViewType ViewType = EGfxViewType::ShaderResource;
    EGfxFormat Format = EGfxFormat::Unknown;
    EGfxTextureViewDimension Dimension = EGfxTextureViewDimension::Inherit;
    FGfxSubresourceRange Range = FGfxSubresourceRange::All();
};

enum class EGfxFilter : uint8
{
    Point,
    Linear,
};

enum class EGfxAddressMode : uint8
{
    Wrap,
    Mirror,
    Clamp,
    Border,
    MirrorOnce,
};

enum class EGfxBorderColor : uint8
{
    TransparentBlack,
    OpaqueBlack,
    OpaqueWhite,
};

enum class EGfxCompareOp : uint8
{
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
};

enum class EGfxSamplerReduction : uint8
{
    Standard,
    Comparison,
    Minimum,
    Maximum,
};

struct FGfxSamplerDesc
{
    EGfxFilter MinFilter = EGfxFilter::Linear;
    EGfxFilter MagFilter = EGfxFilter::Linear;
    EGfxFilter MipFilter = EGfxFilter::Linear;
    EGfxAddressMode AddressU = EGfxAddressMode::Wrap;
    EGfxAddressMode AddressV = EGfxAddressMode::Wrap;
    EGfxAddressMode AddressW = EGfxAddressMode::Wrap;
    EGfxSamplerReduction Reduction = EGfxSamplerReduction::Standard;
    EGfxCompareOp CompareOp = EGfxCompareOp::Never;
    EGfxBorderColor BorderColor = EGfxBorderColor::TransparentBlack;
    uint8 MaxAnisotropy = 1;
    float32 MipLodBias = 0.0f;
    float32 MinLod = 0.0f;
    float32 MaxLod = 1000.0f;

    bool8 operator==(const FGfxSamplerDesc&) const = default;
};

struct FGfxTextureCopyLayout
{
    uint64 TotalSize = 0;
    uint32 OffsetAlignment = 0;
    uint32 RowPitch = 0;
    uint32 RowSize = 0;
    uint32 RowCount = 0;
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 Depth = 1;
};

struct FGfxTransientAllocation
{
    TSpan<byte8> CpuData;
    FGfxBufferRange GpuRange;

public:
    [[nodiscard]] bool8 IsValid() const
    {
        return !CpuData.empty();
    }
};
