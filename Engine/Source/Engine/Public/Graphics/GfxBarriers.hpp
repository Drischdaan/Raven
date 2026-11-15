// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "GfxTypes.hpp"
#include "Core/CoreTypes.hpp"

enum class EGfxBarrierSync : uint32
{
    None = 0,
    All = 1 << 0,
    Draw = 1 << 1,
    IndexInput = 1 << 2,
    VertexShading = 1 << 3,
    PixelShading = 1 << 4,
    DepthStencil = 1 << 5,
    RenderTarget = 1 << 6,
    ComputeShading = 1 << 7,
    AllShading = 1 << 8,
    Copy = 1 << 9,
    Resolve = 1 << 10,
    ExecuteIndirect = 1 << 11,
    BuildAccelerationStructure = 1 << 12,
    Raytracing = 1 << 13,
};

RAVEN_ENUM_FLAGS(EGfxBarrierSync)

enum class EGfxBarrierAccess : uint32
{
    Common = 0,
    VertexBuffer = 1 << 0,
    ConstantBuffer = 1 << 1,
    IndexBuffer = 1 << 2,
    RenderTarget = 1 << 3,
    UnorderedAccess = 1 << 4,
    DepthStencilWrite = 1 << 5,
    DepthStencilRead = 1 << 6,
    ShaderResource = 1 << 7,
    IndirectArgument = 1 << 8,
    CopySource = 1 << 9,
    CopyDestination = 1 << 10,
    ResolveSource = 1 << 11,
    ResolveDestination = 1 << 12,
    AccelerationStructureRead = 1 << 13,
    AccelerationStructureWrite = 1 << 14,
    NoAccess = 1u << 31,
};

RAVEN_ENUM_FLAGS(EGfxBarrierAccess)

enum class EGfxTextureLayout : uint8
{
    Undefined,
    Common,
    Present,
    RenderTarget,
    DepthStencilWrite,
    DepthStencilRead,
    ShaderResource,
    UnorderedAccess,
    CopySource,
    CopyDestination,
    ResolveSource,
    ResolveDestination,
};

struct FGfxGlobalBarrier
{
    EGfxBarrierSync SyncBefore = EGfxBarrierSync::None;
    EGfxBarrierSync SyncAfter = EGfxBarrierSync::None;
    EGfxBarrierAccess AccessBefore = EGfxBarrierAccess::NoAccess;
    EGfxBarrierAccess AccessAfter = EGfxBarrierAccess::NoAccess;
};

struct FGfxBufferBarrier
{
    FGfxBufferHandle BufferHandle;
    EGfxBarrierSync SyncBefore = EGfxBarrierSync::None;
    EGfxBarrierSync SyncAfter = EGfxBarrierSync::None;
    EGfxBarrierAccess AccessBefore = EGfxBarrierAccess::NoAccess;
    EGfxBarrierAccess AccessAfter = EGfxBarrierAccess::NoAccess;
    uint64 Offset = 0;
    uint64 Size = GfxWholeSize;
};

struct FGfxTextureBarrier
{
    FGfxTextureHandle TextureHandle;
    EGfxBarrierSync SyncBefore = EGfxBarrierSync::None;
    EGfxBarrierSync SyncAfter = EGfxBarrierSync::None;
    EGfxBarrierAccess AccessBefore = EGfxBarrierAccess::NoAccess;
    EGfxBarrierAccess AccessAfter = EGfxBarrierAccess::NoAccess;
    EGfxTextureLayout LayoutBefore = EGfxTextureLayout::Undefined;
    EGfxTextureLayout LayoutAfter = EGfxTextureLayout::Undefined;
    FGfxSubresourceRange Range = FGfxSubresourceRange::All();
    bool8 bDiscard = false;
};
