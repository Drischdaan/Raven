// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "D3D12Include.hpp"
#include "Core/Memory/Containers/Vector.hpp"
#include "Graphics/GfxCommandList.hpp"

class FD3D12GfxDevice;

class RAVEN_API FD3D12GfxCommandList final : public IGfxCommandList
{
public:
    FD3D12GfxCommandList(FD3D12GfxDevice& InDevice, EGfxQueueType InQueueType, TComPtr<ID3D12GraphicsCommandList10> InCommandList);
    ~FD3D12GfxCommandList() override;

public:
    void Begin(TComPtr<ID3D12CommandAllocator> InAllocator);
    void Close();

    [[nodiscard]] TComPtr<ID3D12CommandAllocator> TakeAllocator();

    [[nodiscard]] ID3D12GraphicsCommandList10* GetNative() const;
    [[nodiscard]] EGfxQueueType GetQueueType() const;
    [[nodiscard]] bool8 IsRecording() const;

public:
    void BeginEvent(FStringView Name) override;
    void EndEvent() override;

    void Barrier(TSpan<const FGfxGlobalBarrier> GlobalBarriers, TSpan<const FGfxBufferBarrier> BufferBarriers, TSpan<const FGfxTextureBarrier> TextureBarriers) override;

    void BeginRendering(const FGfxRenderingDesc& RenderingDesc) override;
    void EndRendering() override;

    void SetPipeline(FGfxPipelineHandle PipelineHandle) override;
    void SetViewport(const FGfxViewport& Viewport) override;
    void SetScissor(const FGfxRect& Scissor) override;
    void SetPushConstants(TSpan<const byte8> Data, uint32 Offset) override;
    void SetConstantBuffer(EGfxConstantSlot Slot, FGfxBufferRange Range) override;
    void SetIndexBuffer(FGfxBufferHandle IndexBufferHandle, EGfxIndexType IndexType, uint64 Offset) override;

    void Draw(uint32 VertexCount, uint32 InstanceCount, uint32 FirstVertex, uint32 FirstInstance) override;
    void DrawIndexed(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, uint32 VertexOffset, uint32 FirstInstance) override;
    void DrawIndexedIndirect(FGfxBufferHandle ArgumentsBufferHandle, uint64 ArgumentsOffset, uint32 MaxCount, FGfxBufferHandle CountBufferHandle, uint64 CountOffset) override;

    void Dispatch(uint32 X, uint32 Y, uint32 Z) override;
    void DispatchIndirect(FGfxBufferHandle ArgumentsBufferHandle, uint64 ArgumentsOffset) override;
    void DispatchMesh(uint32 X, uint32 Y, uint32 Z) override;

    void CopyBuffer(FGfxBufferHandle DestinationBufferHandle, uint64 DestinationOffset, FGfxBufferHandle SourceBufferHandle, uint64 SourceOffset, uint64 Size) override;
    void CopyBufferToTexture(FGfxTextureHandle DestinationTextureHandle, uint32 Subresource, FGfxBufferHandle SourceBufferHandle, uint64 SourceOffset) override;
    void CopyTexture(FGfxTextureHandle DestinationTextureHandle, FGfxTextureHandle SourceTextureHandle) override;

    [[nodiscard]] FGfxTransientAllocation AllocateTransient(uint64 Size, uint64 Alignment) override;

private:
    [[nodiscard]] bool8 EnsureScratchDescriptors();

private:
    FD3D12GfxDevice* Device = nullptr;
    EGfxQueueType QueueType = EGfxQueueType::Graphics;
    TComPtr<ID3D12GraphicsCommandList10> CommandList;
    TComPtr<ID3D12CommandAllocator> Allocator;

    bool8 bIsRecording = false;
    bool8 bIsComputePipelineBound = false;

    bool8 bIsInRenderPass = false;
    bool8 bHasScratchDescriptors = false;
    TArray<uint32, GfxMaxRenderTargets> ScratchRtvIndices = {};
    uint32 ScratchDsvIndex = GfxInvalidBindlessIndex;

    TVector<D3D12_GLOBAL_BARRIER> GlobalBarrierScratch;
    TVector<D3D12_BUFFER_BARRIER> BufferBarrierScratch;
    TVector<D3D12_TEXTURE_BARRIER> TextureBarrierScratch;
};
