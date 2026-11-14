// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Core/CoreTypes.hpp"
#include "Core/Utility/EnumFlags.hpp"

inline constexpr uint32 GfxMaxRenderTargets = 8;
inline constexpr uint32 GfxMaxPushConstantsSize = 128;
inline constexpr uint32 GfxRemainingMips = UINT32_MAX;
inline constexpr uint32 GfxRemainingArraySlices = UINT32_MAX;
inline constexpr uint64 GfxWholeSize = UINT64_MAX;
inline constexpr uint32 GfxInvalidBindlessIndex = UINT32_MAX;

template <typename T>
struct TGfxHandle
{
    uint32 Index = UINT32_MAX;
    uint32 Generation = 0;

public:
    [[nodiscard]] bool8 IsValid() const
    {
        return Index != UINT32_MAX;
    }

    auto operator<=>(const TGfxHandle&) const = default;
};

using FGfxBufferHandle = TGfxHandle<struct FGfxBufferTag>;
using FGfxTextureHandle = TGfxHandle<struct FGfxTextureTag>;
using FGfxTextureViewHandle = TGfxHandle<struct FGfxTextureViewTag>;
using FGfxSamplerHandle = TGfxHandle<struct FGfxSamplerTag>;
using FGfxShaderHandle = TGfxHandle<struct FGfxShaderTag>;
using FGfxPipelineHandle = TGfxHandle<struct FGfxPipelineTag>;

enum class EGfxBackend : uint8
{
    Null = 0,
    D3D12 = 1,
    Vulkan = 2,
};

enum class EGfxQueueType : uint8
{
    Graphics,
    Compute,
    Copy,
    Count,
};

enum class EGfxMemoryType : uint8
{
    GpuOnly,
    Upload,
    Readback,
    GpuUpload,
};

enum class EGfxFormat : uint16
{
    Unknown,

    R8_UNORM,
    R8_UINT,
    RG8_UNORM,
    RGBA8_UNORM,
    RGBA8_SRGB,
    BGRA8_UNORM,
    BGRA8_SRGB,

    R16_UNORM,
    R16_UINT,
    R16_FLOAT,
    RG16_FLOAT,
    RGBA16_UNORM,
    RGBA16_FLOAT,

    R32_UINT,
    R32_FLOAT,
    RG32_UINT,
    RG32_FLOAT,
    RGBA32_UINT,
    RGBA32_FLOAT,

    RGB10A2_UNORM,
    RG11B10_FLOAT,
    RGB9E5_FLOAT,

    D16_UNORM,
    D32_FLOAT,
    D24_UNORM_S8_UINT,
    D32_FLOAT_S8_UINT,

    BC1_UNORM,
    BC1_SRGB,
    BC3_UNORM,
    BC3_SRGB,
    BC4_UNORM,
    BC5_UNORM,
    BC6H_UFLOAT,
    BC7_UNORM,
    BC7_SRGB,

    Count,
};

enum class EGfxBufferUsage : uint32
{
    None = 0,
    Vertex = 1 << 0,
    Index = 1 << 1,
    Constant = 1 << 2,
    ShaderResource = 1 << 3,
    UnorderedAccess = 1 << 4,
    Indirect = 1 << 5,
    CopySource = 1 << 6,
    CopyDestination = 1 << 7,
    AccelerationStructure = 1 << 8,
};

RAVEN_ENUM_FLAGS(EGfxBufferUsage)

enum class EGfxTextureUsage : uint32
{
    None = 0,
    ShaderResource = 1 << 0,
    UnorderedAccess = 1 << 1,
    RenderTarget = 1 << 2,
    DepthStencil = 1 << 3,
    CopySource = 1 << 4,
    CopyDestination = 1 << 5,
};

RAVEN_ENUM_FLAGS(EGfxTextureUsage)

enum class EGfxTextureDimension : uint8
{
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube,
};

enum class EGfxTextureViewDimension : uint8
{
    Inherit,
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    Texture3D,
    TextureCube,
    TextureCubeArray,
};

enum class EGfxTextureAspect : uint8
{
    Auto = 0,
    Color = 1 << 0,
    Depth = 1 << 1,
    Stencil = 1 << 2,
};

RAVEN_ENUM_FLAGS(EGfxTextureAspect)

enum class EGfxViewType : uint8
{
    ShaderResource,
    UnorderedAccess,
};

enum class EGfxIndexType : uint8
{
    UINT16,
    UINT32,
};

enum class EGfxConstantSlot : uint8
{
    Frame,
    View,
    Pass,
    Count,
};

struct FGfxSubresourceRange
{
    uint32 BaseMip = 0;
    uint32 MipCount = GfxRemainingMips;
    uint32 BaseArraySlice = 0;
    uint32 ArraySliceCount = GfxRemainingArraySlices;
    EGfxTextureAspect Aspect = EGfxTextureAspect::Auto;

public:
    [[nodiscard]] static constexpr FGfxSubresourceRange All()
    {
        return {};
    }

    [[nodiscard]] static constexpr FGfxSubresourceRange Single(const uint32 Mip, const uint32 ArraySlice = 0)
    {
        return {.BaseMip = Mip, .MipCount = 1, .BaseArraySlice = ArraySlice, .ArraySliceCount = 1};
    }

    bool8 operator==(const FGfxSubresourceRange&) const = default;
};

struct FGfxBufferRange
{
    FGfxBufferHandle BufferHandle;
    uint64 Offset = 0;
    uint64 Size = GfxWholeSize;
};

struct FGfxViewport
{
    float32 X = 0.0f;
    float32 Y = 0.0f;
    float32 Width = 0.0f;
    float32 Height = 0.0f;
    float32 MinDepth = 0.0f;
    float32 MaxDepth = 1.0f;
};

struct FGfxRect
{
    int32 X = 0;
    int32 Y = 0;
    uint32 Width = 0;
    uint32 Height = 0;
};

struct FGfxSyncPoint
{
    EGfxQueueType QueueType;
    uint64 Value = 0;
};
