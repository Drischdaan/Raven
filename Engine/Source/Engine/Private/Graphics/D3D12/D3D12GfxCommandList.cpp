// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12GfxCommandList.hpp"

#include <print>

#include "Graphics/D3D12/D3D12GfxDevice.hpp"

FD3D12GfxCommandList::FD3D12GfxCommandList(FD3D12GfxDevice& InDevice, const EGfxQueueType InQueueType, TComPtr<ID3D12GraphicsCommandList10> InCommandList)
    : Device(&InDevice), QueueType(InQueueType), CommandList(std::move(InCommandList))
{
}

void FD3D12GfxCommandList::Begin(TComPtr<ID3D12CommandAllocator> InAllocator)
{
    Allocator = std::move(InAllocator);
    CommandList->Reset(Allocator.Get(), nullptr);
    bIsRecording = true;
    bIsComputePipelineBound = QueueType == EGfxQueueType::Compute;

    if (QueueType == EGfxQueueType::Copy)
    {
        return;
    }

    ID3D12DescriptorHeap* const Heaps[] = {
        Device->GetDescriptorHeap(ED3D12DescriptorHeapType::Resource).GetNative(),
        Device->GetDescriptorHeap(ED3D12DescriptorHeapType::Sampler).GetNative(),
    };
    CommandList->SetDescriptorHeaps(std::size(Heaps), Heaps);

    ID3D12RootSignature* RootSignature = Device->GetGlobalRootSignature();
    if (QueueType == EGfxQueueType::Graphics)
    {
        CommandList->SetGraphicsRootSignature(RootSignature);
    }
    CommandList->SetComputeRootSignature(RootSignature);
}

void FD3D12GfxCommandList::Close()
{
    if (!bIsRecording)
    {
        return;
    }
    CommandList->Close();
    bIsRecording = false;
}

TComPtr<ID3D12CommandAllocator> FD3D12GfxCommandList::TakeAllocator()
{
    return std::move(Allocator);
}

ID3D12GraphicsCommandList10* FD3D12GfxCommandList::GetNative() const
{
    return CommandList.Get();
}

EGfxQueueType FD3D12GfxCommandList::GetQueueType() const
{
    return QueueType;
}

bool8 FD3D12GfxCommandList::IsRecording() const
{
    return bIsRecording;
}

void FD3D12GfxCommandList::BeginEvent(const FStringView Name)
{
    char Buffer[128];
    const size64 Length = std::min<size64>(Name.size(), sizeof(Buffer) - 1);
    std::memcpy(Buffer, Name.data(), Length);
    Buffer[Length] = '\0';
    CommandList->BeginEvent(1, Buffer, static_cast<uint32>(Length + 1));
}

void FD3D12GfxCommandList::EndEvent()
{
    CommandList->EndEvent();
}

void FD3D12GfxCommandList::Barrier(TSpan<const FGfxGlobalBarrier> GlobalBarriers, TSpan<const FGfxBufferBarrier> BufferBarriers, TSpan<const FGfxTextureBarrier> TextureBarriers)
{
}

void FD3D12GfxCommandList::BeginRendering(const FGfxRenderingDesc& RenderingDesc)
{
}

void FD3D12GfxCommandList::EndRendering()
{
}

void FD3D12GfxCommandList::SetPipeline(FGfxPipelineHandle PipelineHandle)
{
}

void FD3D12GfxCommandList::SetViewport(const FGfxViewport& Viewport)
{
    const D3D12_VIEWPORT NativeViewport = {
        .TopLeftX = Viewport.X,
        .TopLeftY = Viewport.Y,
        .Width = Viewport.Width,
        .Height = Viewport.Height,
        .MinDepth = Viewport.MinDepth,
        .MaxDepth = Viewport.MaxDepth,
    };
    CommandList->RSSetViewports(1, &NativeViewport);
}

void FD3D12GfxCommandList::SetScissor(const FGfxRect& Scissor)
{
    const D3D12_RECT NativeRect = {
        .left = Scissor.X,
        .top = Scissor.Y,
        .right = Scissor.X + static_cast<int32>(Scissor.Width),
        .bottom = Scissor.Y + static_cast<int32>(Scissor.Height),
    };
    CommandList->RSSetScissorRects(1, &NativeRect);
}

void FD3D12GfxCommandList::SetPushConstants(const TSpan<const byte8> Data, uint32 Offset)
{
    if (Data.size() % sizeof(uint32) != 0 || Offset % sizeof(uint32) != 0 || Offset + Data.size() > GfxMaxPushConstantsSize)
    {
        std::println("SetPushConstants: size ({}) and offset ({}) must be 4-byte aligned and fit in {} bytes", Data.size(), Offset, GfxMaxPushConstantsSize);
        return;
    }
    if (QueueType == EGfxQueueType::Copy)
    {
        std::println("SetPushConstants: not allowed on copy command lists");
        return;
    }

    constexpr uint32 RootIndex = static_cast<uint32>(ED3D12RootParameter::PushConstants);
    const uint32 ValueCount = static_cast<uint32>(Data.size() / sizeof(uint32));
    const uint32 DestinationOffset = Offset / sizeof(uint32);
    if (bIsComputePipelineBound)
    {
        CommandList->SetComputeRoot32BitConstants(RootIndex, ValueCount, Data.data(), DestinationOffset);
    }
    else
    {
        CommandList->SetGraphicsRoot32BitConstants(RootIndex, ValueCount, Data.data(), DestinationOffset);
    }
}

void FD3D12GfxCommandList::SetConstantBuffer(EGfxConstantSlot Slot, FGfxBufferRange Range)
{
}

void FD3D12GfxCommandList::SetIndexBuffer(FGfxBufferHandle IndexBufferHandle, EGfxIndexType IndexType, uint64 Offset)
{
}

void FD3D12GfxCommandList::Draw(const uint32 VertexCount, const uint32 InstanceCount, const uint32 FirstVertex, const uint32 FirstInstance)
{
    CommandList->DrawInstanced(VertexCount, InstanceCount, FirstVertex, FirstInstance);
}

void FD3D12GfxCommandList::DrawIndexed(const uint32 IndexCount, const uint32 InstanceCount, const uint32 FirstIndex, const uint32 VertexOffset, const uint32 FirstInstance)
{
    CommandList->DrawIndexedInstanced(IndexCount, InstanceCount, FirstIndex, static_cast<INT>(VertexOffset), FirstInstance);
}

void FD3D12GfxCommandList::DrawIndexedIndirect(FGfxBufferHandle ArgumentsBufferHandle, uint64 ArgumentsOffset, uint32 MaxCount, FGfxBufferHandle CountBufferHandle, uint64 CountOffset)
{
}

void FD3D12GfxCommandList::Dispatch(const uint32 X, const uint32 Y, const uint32 Z)
{
    CommandList->Dispatch(X, Y, Z);
}

void FD3D12GfxCommandList::DispatchIndirect(FGfxBufferHandle ArgumentsBufferHandle, uint64 ArgumentsOffset)
{
}

void FD3D12GfxCommandList::DispatchMesh(const uint32 X, const uint32 Y, const uint32 Z)
{
    CommandList->DispatchMesh(X, Y, Z);
}

void FD3D12GfxCommandList::CopyBuffer(FGfxBufferHandle DestinationBufferHandle, uint64 DestinationOffset, FGfxBufferHandle SourceBufferHandle, uint64 SourceOffset, uint64 Size)
{
}

void FD3D12GfxCommandList::CopyBufferToTexture(FGfxTextureHandle DestinationTextureHandle, uint32 Subresource, FGfxBufferHandle SourceBufferHandle, uint64 SourceOffset)
{
}

void FD3D12GfxCommandList::CopyTexture(FGfxTextureHandle DestinationTextureHandle, FGfxTextureHandle SourceTextureHandle)
{
}

FGfxTransientAllocation FD3D12GfxCommandList::AllocateTransient(uint64 Size, uint64 Alignment)
{
    return {};
}
