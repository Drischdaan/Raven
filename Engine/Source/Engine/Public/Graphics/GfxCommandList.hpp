// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <optional>

#include "GfxBarriers.hpp"
#include "GfxResources.hpp"
#include "GfxTypes.hpp"
#include "Core/Memory/Containers/Array.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Memory/Containers/String.hpp"

enum class EGfxLoadOp : uint8
{
    Load,
    Clear,
    Discard,
};

enum class EGfxStoreOp : uint8
{
    Store,
    Discard,
};

struct FGfxRenderTargetAttachment
{
    FGfxTextureHandle TextureHandle;
    uint32 MipLevel = 0;
    uint32 ArraySlice = 0;
    EGfxLoadOp LoadOp = EGfxLoadOp::Load;
    EGfxStoreOp StoreOp = EGfxStoreOp::Store;
    TArray<float32, 4> ClearColor = {};
};

struct FGfxDepthStencilAttachment
{
    FGfxTextureHandle TextureHandle;
    uint32 MipLevel = 0;
    uint32 ArraySlice = 0;
    EGfxLoadOp DepthLoadOp = EGfxLoadOp::Load;
    EGfxStoreOp DepthStoreOp = EGfxStoreOp::Store;
    EGfxLoadOp StencilLoadOp = EGfxLoadOp::Discard;
    EGfxStoreOp StencilStoreOp = EGfxStoreOp::Discard;
    float32 ClearDepth = 0.0f;
    uint8 ClearStencil = 0;
    bool8 bReadOnly = false;
};

struct FGfxRenderingDesc
{
    TSpan<const FGfxRenderTargetAttachment> RenderTargets; // <= GfxMaxRenderTargets, only read during the call
    std::optional<FGfxDepthStencilAttachment> DepthStencil;
};

class RAVEN_API IGfxCommandList
{
public:
    virtual ~IGfxCommandList() = default;

public:
    virtual void BeginEvent(FStringView Name) = 0;
    virtual void EndEvent() = 0;

    virtual void Barrier(TSpan<const FGfxGlobalBarrier> GlobalBarriers, TSpan<const FGfxBufferBarrier> BufferBarriers, TSpan<const FGfxTextureBarrier> TextureBarriers) = 0;

    virtual void BeginRendering(const FGfxRenderingDesc& RenderingDesc) = 0;
    virtual void EndRendering() = 0;

    virtual void SetPipeline(FGfxPipelineHandle PipelineHandle) = 0;
    virtual void SetViewport(const FGfxViewport& Viewport) = 0;
    virtual void SetScissor(const FGfxRect& Scissor) = 0;
    virtual void SetPushConstants(TSpan<const byte8> Data, uint32 Offset = 0) = 0;
    virtual void SetConstantBuffer(EGfxConstantSlot Slot, FGfxBufferRange Range) = 0;
    virtual void SetIndexBuffer(FGfxBufferHandle IndexBufferHandle, EGfxIndexType IndexType, uint64 Offset = 0) = 0;

    virtual void Draw(uint32 VertexCount, uint32 InstanceCount = 1, uint32 FirstVertex = 0, uint32 FirstInstance = 0) = 0;
    virtual void DrawIndexed(uint32 IndexCount, uint32 InstanceCount = 1, uint32 FirstIndex = 0, uint32 VertexOffset = 0, uint32 FirstInstance = 0) = 0;
    virtual void DrawIndexedIndirect(FGfxBufferHandle ArgumentsBufferHandle, uint64 ArgumentsOffset, uint32 MaxCount, FGfxBufferHandle CountBufferHandle = {}, uint64 CountOffset = 0) = 0;
    virtual void Dispatch(uint32 X, uint32 Y, uint32 Z) = 0;
    virtual void DispatchIndirect(FGfxBufferHandle ArgumentsBufferHandle, uint64 ArgumentsOffset) = 0;
    virtual void DispatchMesh(uint32 X, uint32 Y, uint32 Z) = 0;

    virtual void CopyBuffer(FGfxBufferHandle DestinationBufferHandle, uint64 DestinationOffset, FGfxBufferHandle SourceBufferHandle, uint64 SourceOffset, uint64 Size) = 0;
    virtual void CopyBufferToTexture(FGfxTextureHandle DestinationTextureHandle, uint32 Subresource, FGfxBufferHandle SourceBufferHandle, uint64 SourceOffset) = 0;
    virtual void CopyTexture(FGfxTextureHandle DestinationTextureHandle, FGfxTextureHandle SourceTextureHandle) = 0;

    [[nodiscard]] virtual FGfxTransientAllocation AllocateTransient(uint64 Size, uint64 Alignment = 256) = 0;
};
