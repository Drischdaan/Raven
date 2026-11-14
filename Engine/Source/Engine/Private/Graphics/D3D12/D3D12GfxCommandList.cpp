// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12GfxCommandList.hpp"

#include <print>

#include "Graphics/D3D12/D3D12Conversions.hpp"
#include "Graphics/D3D12/D3D12GfxDevice.hpp"

namespace
{
    D3D12_RENDER_TARGET_VIEW_DESC MakeRtvDesc(const FGfxTextureDesc& Desc, const DXGI_FORMAT Format, const uint32 MipLevel, const uint32 ArraySlice)
    {
        const uint32 ArraySize = GetTextureArraySize(Desc);
        const bool8 bIsMultisampled = Desc.SampleCount > 1;

        D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
        RtvDesc.Format = Format;
        switch (Desc.Dimension)
        {
        case EGfxTextureDimension::Texture1D:
            if (ArraySize > 1)
            {
                RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                RtvDesc.Texture1DArray = {.MipSlice = MipLevel, .FirstArraySlice = ArraySlice, .ArraySize = 1};
            }
            else
            {
                RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                RtvDesc.Texture1D = {.MipSlice = MipLevel};
            }
            break;
        case EGfxTextureDimension::Texture3D:
            RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
            RtvDesc.Texture3D = {.MipSlice = MipLevel, .FirstWSlice = ArraySlice, .WSize = 1};
            break;
        default:
            if (bIsMultisampled)
            {
                RtvDesc.ViewDimension = ArraySize > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                RtvDesc.Texture2DMSArray = {.FirstArraySlice = ArraySlice, .ArraySize = 1};
            }
            else if (ArraySize > 1)
            {
                RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                RtvDesc.Texture2DArray = {.MipSlice = MipLevel, .FirstArraySlice = ArraySlice, .ArraySize = 1, .PlaneSlice = 0};
            }
            else
            {
                RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                RtvDesc.Texture2D = {.MipSlice = MipLevel, .PlaneSlice = 0};
            }
            break;
        }
        return RtvDesc;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC MakeDsvDesc(const FGfxTextureDesc& Desc, const DXGI_FORMAT Format, const uint32 MipLevel, const uint32 ArraySlice, const D3D12_DSV_FLAGS Flags)
    {
        const uint32 ArraySize = GetTextureArraySize(Desc);
        const bool8 bIsMultisampled = Desc.SampleCount > 1;

        D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc = {};
        DsvDesc.Format = Format;
        DsvDesc.Flags = Flags;
        if (Desc.Dimension == EGfxTextureDimension::Texture1D)
        {
            if (ArraySize > 1)
            {
                DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                DsvDesc.Texture1DArray = {.MipSlice = MipLevel, .FirstArraySlice = ArraySlice, .ArraySize = 1};
            }
            else
            {
                DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                DsvDesc.Texture1D = {.MipSlice = MipLevel};
            }
        }
        else if (bIsMultisampled)
        {
            DsvDesc.ViewDimension = ArraySize > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D12_DSV_DIMENSION_TEXTURE2DMS;
            DsvDesc.Texture2DMSArray = {.FirstArraySlice = ArraySlice, .ArraySize = 1};
        }
        else if (ArraySize > 1)
        {
            DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            DsvDesc.Texture2DArray = {.MipSlice = MipLevel, .FirstArraySlice = ArraySlice, .ArraySize = 1};
        }
        else
        {
            DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            DsvDesc.Texture2D = {.MipSlice = MipLevel};
        }
        return DsvDesc;
    }
}

FD3D12GfxCommandList::FD3D12GfxCommandList(FD3D12GfxDevice& InDevice, const EGfxQueueType InQueueType, TComPtr<ID3D12GraphicsCommandList10> InCommandList)
    : Device(&InDevice), QueueType(InQueueType), CommandList(std::move(InCommandList))
{
    ScratchRtvIndices.fill(GfxInvalidBindlessIndex);
}

FD3D12GfxCommandList::~FD3D12GfxCommandList()
{
    if (bHasScratchDescriptors)
    {
        for (const uint32 Index : ScratchRtvIndices)
        {
            Device->GetDescriptorHeap(ED3D12DescriptorHeapType::RenderTarget).Free(Index);
        }
        Device->GetDescriptorHeap(ED3D12DescriptorHeapType::DepthStencil).Free(ScratchDsvIndex);
    }
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

    bIsInRenderPass = false;
}

void FD3D12GfxCommandList::Close()
{
    if (!bIsRecording)
    {
        return;
    }
    if (bIsInRenderPass)
    {
        std::println("Command list closed inside a render pass; ending it");
        CommandList->EndRenderPass();
        bIsInRenderPass = false;
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

void FD3D12GfxCommandList::Barrier(const TSpan<const FGfxGlobalBarrier> GlobalBarriers, const TSpan<const FGfxBufferBarrier> BufferBarriers, const TSpan<const FGfxTextureBarrier> TextureBarriers)
{
    GlobalBarrierScratch.clear();
    BufferBarrierScratch.clear();
    TextureBarrierScratch.clear();

    for (const FGfxGlobalBarrier& Barrier : GlobalBarriers)
    {
        GlobalBarrierScratch.push_back(D3D12_GLOBAL_BARRIER{
            .SyncBefore = ToD3D12BarrierSync(Barrier.SyncBefore),
            .SyncAfter = ToD3D12BarrierSync(Barrier.SyncAfter),
            .AccessBefore = ToD3D12BarrierAccess(Barrier.AccessBefore),
            .AccessAfter = ToD3D12BarrierAccess(Barrier.AccessAfter),
        });
    }

    for (const FGfxBufferBarrier& Barrier : BufferBarriers)
    {
        const FD3D12Buffer* Buffer = Device->GetBuffer(Barrier.BufferHandle);
        if (Buffer == nullptr)
        {
            std::println("Barrier: invalid or stale buffer handle");
            continue;
        }
        BufferBarrierScratch.push_back(D3D12_BUFFER_BARRIER{
            .SyncBefore = ToD3D12BarrierSync(Barrier.SyncBefore),
            .SyncAfter = ToD3D12BarrierSync(Barrier.SyncAfter),
            .AccessBefore = ToD3D12BarrierAccess(Barrier.AccessBefore),
            .AccessAfter = ToD3D12BarrierAccess(Barrier.AccessAfter),
            .pResource = Buffer->Resource.Get(),
            .Offset = 0,
            .Size = UINT64_MAX,
        });
    }

    for (const FGfxTextureBarrier& Barrier : TextureBarriers)
    {
        const FD3D12Texture* Texture = Device->GetTexture(Barrier.TextureHandle);
        if (Texture == nullptr)
        {
            std::println("Barrier: invalid or stale texture handle");
            continue;
        }

        D3D12_BARRIER_SUBRESOURCE_RANGE Subresources = {};
        if (Barrier.Range == FGfxSubresourceRange::All())
        {
            Subresources.IndexOrFirstMipLevel = UINT32_MAX;
        }
        else
        {
            TResult<FGfxSubresourceRange> Range = ResolveSubresourceRange(Texture->Desc, Barrier.Range);
            if (Range.HasError())
            {
                std::println("Barrier: {}", Range.GetError().Message);
                continue;
            }
            Subresources = ToD3D12BarrierRange(Texture->Desc, Range.GetValue());
        }

        TextureBarrierScratch.push_back(D3D12_TEXTURE_BARRIER{
            .SyncBefore = ToD3D12BarrierSync(Barrier.SyncBefore),
            .SyncAfter = ToD3D12BarrierSync(Barrier.SyncAfter),
            .AccessBefore = ToD3D12BarrierAccess(Barrier.AccessBefore),
            .AccessAfter = ToD3D12BarrierAccess(Barrier.AccessAfter),
            .LayoutBefore = ToD3D12BarrierLayout(Barrier.LayoutBefore),
            .LayoutAfter = ToD3D12BarrierLayout(Barrier.LayoutAfter),
            .pResource = Texture->Resource.Get(),
            .Subresources = Subresources,
            .Flags = Barrier.bDiscard ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE,
        });
    }

    D3D12_BARRIER_GROUP Groups[3] = {};
    uint32 GroupCount = 0;
    if (!GlobalBarrierScratch.empty())
    {
        D3D12_BARRIER_GROUP& Group = Groups[GroupCount++];
        Group.Type = D3D12_BARRIER_TYPE_GLOBAL;
        Group.NumBarriers = static_cast<uint32>(GlobalBarrierScratch.size());
        Group.pGlobalBarriers = GlobalBarrierScratch.data();
    }
    if (!BufferBarrierScratch.empty())
    {
        D3D12_BARRIER_GROUP& Group = Groups[GroupCount++];
        Group.Type = D3D12_BARRIER_TYPE_BUFFER;
        Group.NumBarriers = static_cast<uint32>(BufferBarrierScratch.size());
        Group.pBufferBarriers = BufferBarrierScratch.data();
    }
    if (!TextureBarrierScratch.empty())
    {
        D3D12_BARRIER_GROUP& Group = Groups[GroupCount++];
        Group.Type = D3D12_BARRIER_TYPE_TEXTURE;
        Group.NumBarriers = static_cast<uint32>(TextureBarrierScratch.size());
        Group.pTextureBarriers = TextureBarrierScratch.data();
    }
    if (GroupCount > 0)
    {
        CommandList->Barrier(GroupCount, Groups);
    }
}

void FD3D12GfxCommandList::BeginRendering(const FGfxRenderingDesc& RenderingDesc)
{
    if (QueueType != EGfxQueueType::Graphics)
    {
        std::println("BeginRendering: only valid on graphics command lists");
        return;
    }
    if (bIsInRenderPass)
    {
        std::println("BeginRendering: already inside a render pass");
        return;
    }
    if (RenderingDesc.RenderTargets.size() > GfxMaxRenderTargets)
    {
        std::println("BeginRendering: at most {} render targets", GfxMaxRenderTargets);
        return;
    }
    if (!EnsureScratchDescriptors())
    {
        return;
    }

    ID3D12Device15* NativeDevice = Device->GetNativeDevice();
    FD3D12DescriptorHeap& RtvHeap = Device->GetDescriptorHeap(ED3D12DescriptorHeapType::RenderTarget);
    FD3D12DescriptorHeap& DsvHeap = Device->GetDescriptorHeap(ED3D12DescriptorHeapType::DepthStencil);

    D3D12_RENDER_PASS_RENDER_TARGET_DESC RenderTargets[GfxMaxRenderTargets] = {};
    uint32 RenderTargetCount = 0;

    for (const FGfxRenderTargetAttachment& Attachment : RenderingDesc.RenderTargets)
    {
        const FD3D12Texture* Texture = Device->GetTexture(Attachment.TextureHandle);
        if (Texture == nullptr)
        {
            std::println("BeginRendering: invalid or stale render target handle");
            return;
        }
        const DXGI_FORMAT Format = GetD3D12FormatInfo(Texture->Desc.Format).ViewFormat;

        const D3D12_CPU_DESCRIPTOR_HANDLE Rtv = RtvHeap.GetDescriptor(ScratchRtvIndices[RenderTargetCount]).CpuHandle;
        const D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = MakeRtvDesc(Texture->Desc, Format, Attachment.MipLevel, Attachment.ArraySlice);
        NativeDevice->CreateRenderTargetView(Texture->Resource.Get(), &RtvDesc, Rtv);

        D3D12_RENDER_PASS_RENDER_TARGET_DESC& Target = RenderTargets[RenderTargetCount++];
        Target.cpuDescriptor = Rtv;
        Target.BeginningAccess.Type = ToD3D12BeginningAccess(Attachment.LoadOp);
        if (Attachment.LoadOp == EGfxLoadOp::Clear)
        {
            Target.BeginningAccess.Clear.ClearValue.Format = Format;
            std::memcpy(Target.BeginningAccess.Clear.ClearValue.Color, Attachment.ClearColor.data(), sizeof(float32) * 4);
        }
        Target.EndingAccess.Type = ToD3D12EndingAccess(Attachment.StoreOp);
    }

    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC DepthStencil = {};
    D3D12_RENDER_PASS_FLAGS Flags = D3D12_RENDER_PASS_FLAG_NONE;

    if (RenderingDesc.DepthStencil.has_value())
    {
        const FGfxDepthStencilAttachment& Attachment = *RenderingDesc.DepthStencil;
        const FD3D12Texture* Texture = Device->GetTexture(Attachment.TextureHandle);
        if (Texture == nullptr)
        {
            std::println("BeginRendering: invalid or stale depth stencil handle");
            return;
        }
        const FD3D12FormatInfo FormatInfo = GetD3D12FormatInfo(Texture->Desc.Format);

        D3D12_DSV_FLAGS DsvFlags = D3D12_DSV_FLAG_NONE;
        if (Attachment.bReadOnly)
        {
            DsvFlags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
            Flags |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_DEPTH;
            if (FormatInfo.bHasStencil)
            {
                DsvFlags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
                Flags |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_STENCIL;
            }
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE Dsv = DsvHeap.GetDescriptor(ScratchDsvIndex).CpuHandle;
        const D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc = MakeDsvDesc(Texture->Desc, FormatInfo.DepthStencilFormat, Attachment.MipLevel, Attachment.ArraySlice, DsvFlags);
        NativeDevice->CreateDepthStencilView(Texture->Resource.Get(), &DsvDesc, Dsv);

        DepthStencil.cpuDescriptor = Dsv;
        DepthStencil.DepthBeginningAccess.Type = ToD3D12BeginningAccess(Attachment.DepthLoadOp);
        DepthStencil.DepthEndingAccess.Type = ToD3D12EndingAccess(Attachment.DepthStoreOp);
        if (Attachment.DepthLoadOp == EGfxLoadOp::Clear)
        {
            DepthStencil.DepthBeginningAccess.Clear.ClearValue.Format = FormatInfo.DepthStencilFormat;
            DepthStencil.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = Attachment.ClearDepth;
        }

        if (FormatInfo.bHasStencil)
        {
            DepthStencil.StencilBeginningAccess.Type = ToD3D12BeginningAccess(Attachment.StencilLoadOp);
            DepthStencil.StencilEndingAccess.Type = ToD3D12EndingAccess(Attachment.StencilStoreOp);
            if (Attachment.StencilLoadOp == EGfxLoadOp::Clear)
            {
                DepthStencil.StencilBeginningAccess.Clear.ClearValue.Format = FormatInfo.DepthStencilFormat;
                DepthStencil.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = Attachment.ClearStencil;
            }
        }
        else
        {
            DepthStencil.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
            DepthStencil.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
        }
    }

    CommandList->BeginRenderPass(RenderTargetCount, RenderTargets, RenderingDesc.DepthStencil.has_value() ? &DepthStencil : nullptr, Flags);
    bIsInRenderPass = true;
}

void FD3D12GfxCommandList::EndRendering()
{
    if (!bIsInRenderPass)
    {
        std::println("EndRendering: no render pass is active");
        return;
    }
    CommandList->EndRenderPass();
    bIsInRenderPass = false;
}

void FD3D12GfxCommandList::SetPipeline(const FGfxPipelineHandle PipelineHandle)
{
    const FD3D12Pipeline* Pipeline = Device->GetPipeline(PipelineHandle);
    if (Pipeline == nullptr)
    {
        std::println("SetPipeline: invalid or stale pipeline handle");
        return;
    }
    if (QueueType == EGfxQueueType::Copy)
    {
        std::println("SetPipeline: not allowed on copy command lists");
        return;
    }
    if (Pipeline->Type != ED3D12PipelineType::Compute && QueueType != EGfxQueueType::Graphics)
    {
        std::println("SetPipeline: graphics pipelines need a graphics command list");
        return;
    }

    CommandList->SetPipelineState(Pipeline->PipelineState.Get());
    bIsComputePipelineBound = Pipeline->Type == ED3D12PipelineType::Compute;
    if (Pipeline->Type == ED3D12PipelineType::Graphics)
    {
        CommandList->IASetPrimitiveTopology(Pipeline->Topology);
    }
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

void FD3D12GfxCommandList::SetConstantBuffer(const EGfxConstantSlot Slot, const FGfxBufferRange Range)
{
    if (QueueType == EGfxQueueType::Copy)
    {
        std::println("SetConstantBuffer: not allowed on copy command lists");
        return;
    }
    const FD3D12Buffer* Buffer = Device->GetBuffer(Range.BufferHandle);
    if (Buffer == nullptr || Range.Offset >= Buffer->Desc.Size)
    {
        std::println("SetConstantBuffer: invalid buffer handle or offset");
        return;
    }
    if (Range.Offset % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT != 0)
    {
        std::println("SetConstantBuffer: offset must be a multiple of 256");
        return;
    }

    const uint32 RootIndex = static_cast<uint32>(ToD3D12RootParameter(Slot));
    const D3D12_GPU_VIRTUAL_ADDRESS Address = Buffer->GpuAddress + Range.Offset;
    if (bIsComputePipelineBound)
    {
        CommandList->SetComputeRootConstantBufferView(RootIndex, Address);
    }
    else
    {
        CommandList->SetGraphicsRootConstantBufferView(RootIndex, Address);
    }
}

void FD3D12GfxCommandList::SetIndexBuffer(const FGfxBufferHandle IndexBufferHandle, const EGfxIndexType IndexType, const uint64 Offset)
{
    const FD3D12Buffer* Buffer = Device->GetBuffer(IndexBufferHandle);
    if (Buffer == nullptr || Offset >= Buffer->Desc.Size)
    {
        std::println("SetIndexBuffer: invalid buffer handle or offset");
        return;
    }
    const D3D12_INDEX_BUFFER_VIEW View = {
        .BufferLocation = Buffer->GpuAddress + Offset,
        .SizeInBytes = static_cast<uint32>(std::min<uint64>(Buffer->Desc.Size - Offset, UINT32_MAX)),
        .Format = IndexType == EGfxIndexType::UINT16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
    };
    CommandList->IASetIndexBuffer(&View);
}

void FD3D12GfxCommandList::Draw(const uint32 VertexCount, const uint32 InstanceCount, const uint32 FirstVertex, const uint32 FirstInstance)
{
    CommandList->DrawInstanced(VertexCount, InstanceCount, FirstVertex, FirstInstance);
}

void FD3D12GfxCommandList::DrawIndexed(const uint32 IndexCount, const uint32 InstanceCount, const uint32 FirstIndex, const uint32 VertexOffset, const uint32 FirstInstance)
{
    CommandList->DrawIndexedInstanced(IndexCount, InstanceCount, FirstIndex, static_cast<INT>(VertexOffset), FirstInstance);
}

void FD3D12GfxCommandList::DrawIndexedIndirect(const FGfxBufferHandle ArgumentsBufferHandle, const uint64 ArgumentsOffset, const uint32 MaxCount, const FGfxBufferHandle CountBufferHandle,
                                               const uint64 CountOffset)
{
    const FD3D12Buffer* Arguments = Device->GetBuffer(ArgumentsBufferHandle);
    if (Arguments == nullptr)
    {
        std::println("DrawIndexedIndirect: invalid or stale arguments buffer");
        return;
    }
    const FD3D12Buffer* Count = CountBufferHandle.IsValid() ? Device->GetBuffer(CountBufferHandle) : nullptr;
    if (CountBufferHandle.IsValid() && Count == nullptr)
    {
        std::println("DrawIndexedIndirect: invalid or stale count buffer");
        return;
    }
    CommandList->ExecuteIndirect(Device->GetDrawIndexedSignature(), MaxCount, Arguments->Resource.Get(), ArgumentsOffset, Count != nullptr ? Count->Resource.Get() : nullptr, CountOffset);
}

void FD3D12GfxCommandList::Dispatch(const uint32 X, const uint32 Y, const uint32 Z)
{
    CommandList->Dispatch(X, Y, Z);
}

void FD3D12GfxCommandList::DispatchIndirect(const FGfxBufferHandle ArgumentsBufferHandle, const uint64 ArgumentsOffset)
{
    const FD3D12Buffer* Arguments = Device->GetBuffer(ArgumentsBufferHandle);
    if (Arguments == nullptr)
    {
        std::println("DispatchIndirect: invalid or stale arguments buffer");
        return;
    }
    CommandList->ExecuteIndirect(Device->GetDispatchSignature(), 1, Arguments->Resource.Get(), ArgumentsOffset, nullptr, 0);
}

void FD3D12GfxCommandList::DispatchMesh(const uint32 X, const uint32 Y, const uint32 Z)
{
    CommandList->DispatchMesh(X, Y, Z);
}

void FD3D12GfxCommandList::CopyBuffer(const FGfxBufferHandle DestinationBufferHandle, const uint64 DestinationOffset, const FGfxBufferHandle SourceBufferHandle, const uint64 SourceOffset,
                                      const uint64 Size)
{
    const FD3D12Buffer* Destination = Device->GetBuffer(DestinationBufferHandle);
    const FD3D12Buffer* Source = Device->GetBuffer(SourceBufferHandle);
    if (Destination == nullptr || Source == nullptr)
    {
        std::println("CopyBuffer: invalid or stale buffer handle");
        return;
    }
    if (DestinationOffset + Size > Destination->Desc.Size || SourceOffset + Size > Source->Desc.Size)
    {
        std::println("CopyBuffer: range out of bounds");
        return;
    }
    CommandList->CopyBufferRegion(Destination->Resource.Get(), DestinationOffset, Source->Resource.Get(), SourceOffset, Size);
}

void FD3D12GfxCommandList::CopyBufferToTexture(const FGfxTextureHandle DestinationTextureHandle, const uint32 Subresource, const FGfxBufferHandle SourceBufferHandle, const uint64 SourceOffset)
{
    const FD3D12Texture* Destination = Device->GetTexture(DestinationTextureHandle);
    const FD3D12Buffer* Source = Device->GetBuffer(SourceBufferHandle);
    if (Destination == nullptr || Source == nullptr)
    {
        std::println("CopyBufferToTexture: invalid or stale handle");
        return;
    }
    if (SourceOffset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT != 0)
    {
        std::println("CopyBufferToTexture: source offset must be a multiple of 512");
        return;
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
    uint64 TotalSize = 0;
    Device->GetNativeDevice()->GetCopyableFootprints1(&Destination->NativeDesc, Subresource, 1, SourceOffset, &Footprint, nullptr, nullptr, &TotalSize);
    if (TotalSize == UINT64_MAX || SourceOffset + TotalSize > Source->Desc.Size)
    {
        std::println("CopyBufferToTexture: invalid subresource or source buffer too small");
        return;
    }

    D3D12_TEXTURE_COPY_LOCATION DestinationLocation = {};
    DestinationLocation.pResource = Destination->Resource.Get();
    DestinationLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    DestinationLocation.SubresourceIndex = Subresource;

    D3D12_TEXTURE_COPY_LOCATION SourceLocation = {};
    SourceLocation.pResource = Source->Resource.Get();
    SourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    SourceLocation.PlacedFootprint = Footprint;

    CommandList->CopyTextureRegion(&DestinationLocation, 0, 0, 0, &SourceLocation, nullptr);
}

void FD3D12GfxCommandList::CopyTexture(const FGfxTextureHandle DestinationTextureHandle, const FGfxTextureHandle SourceTextureHandle)
{
    const FD3D12Texture* Destination = Device->GetTexture(DestinationTextureHandle);
    const FD3D12Texture* Source = Device->GetTexture(SourceTextureHandle);
    if (Destination == nullptr || Source == nullptr)
    {
        std::println("CopyTexture: invalid or stale texture handle");
        return;
    }
    CommandList->CopyResource(Destination->Resource.Get(), Source->Resource.Get());
}

FGfxTransientAllocation FD3D12GfxCommandList::AllocateTransient(const uint64 Size, const uint64 Alignment)
{
    return Device->AllocateTransient(Size, Alignment);
}

bool8 FD3D12GfxCommandList::EnsureScratchDescriptors()
{
    if (bHasScratchDescriptors)
    {
        return true;
    }
    FD3D12DescriptorHeap& RtvHeap = Device->GetDescriptorHeap(ED3D12DescriptorHeapType::RenderTarget);
    FD3D12DescriptorHeap& DsvHeap = Device->GetDescriptorHeap(ED3D12DescriptorHeapType::DepthStencil);

    const auto FreeAll = [&]
    {
        for (uint32& Index : ScratchRtvIndices)
        {
            RtvHeap.Free(Index);
            Index = GfxInvalidBindlessIndex;
        }
        DsvHeap.Free(ScratchDsvIndex);
        ScratchDsvIndex = GfxInvalidBindlessIndex;
    };

    for (uint32& Index : ScratchRtvIndices)
    {
        TResult<FD3D12Descriptor> Rtv = RtvHeap.Allocate();
        if (Rtv.HasError())
        {
            std::println("BeginRendering: {}", Rtv.GetError().Message);
            FreeAll();
            return false;
        }
        Index = Rtv.GetValue().Index;
    }
    TResult<FD3D12Descriptor> Dsv = DsvHeap.Allocate();
    if (Dsv.HasError())
    {
        std::println("BeginRendering: {}", Dsv.GetError().Message);
        FreeAll();
        return false;
    }
    ScratchDsvIndex = Dsv.GetValue().Index;
    bHasScratchDescriptors = true;
    return true;
}
