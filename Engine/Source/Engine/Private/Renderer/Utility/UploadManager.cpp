// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Utility/UploadManager.hpp"

#include <algorithm>

#include "Graphics/GfxBarriers.hpp"
#include "Graphics/GfxCommandList.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Renderer/RendererLog.hpp"

namespace
{
    constexpr uint64 TextureDataAlignment = 512;
}

void FUploadManager::Initialize(IGfxDevice& InDevice)
{
    Device = &InDevice;
}

uint64 FUploadManager::Reserve(const uint64 Size, const uint64 Alignment)
{
    const uint64 Offset = (PendingData.size() + Alignment - 1) & ~(Alignment - 1);
    PendingData.resize(Offset + Size);
    return Offset;
}

void FUploadManager::UploadBuffer(const FGfxBufferHandle Destination, const uint64 DestinationOffset, const TSpan<const byte8> Data)
{
    if (Data.empty())
    {
        return;
    }
    const uint64 DataOffset = Reserve(Data.size(), 16);
    std::memcpy(PendingData.data() + DataOffset, Data.data(), Data.size());
    PendingBufferUploads.push_back({.Destination = Destination, .DestinationOffset = DestinationOffset, .DataOffset = DataOffset, .Size = Data.size()});
}

FResult FUploadManager::UploadTexture(const FGfxTextureHandle Destination, const uint32 Subresource, const TSpan<const byte8> Data)
{
    const FGfxTextureCopyLayout Layout = Device->GetCopyLayout(Destination, Subresource);
    if (Layout.TotalSize == 0)
    {
        return RESULT_ERROR("UploadTexture: invalid texture or subresource");
    }

    const uint64 SourceSize = static_cast<uint64>(Layout.RowSize) * Layout.RowCount * Layout.Depth;
    if (Data.size() < SourceSize)
    {
        return RESULT_ERROR(String::Format("UploadTexture: subresource {} needs {} bytes, got {}", Subresource, SourceSize, Data.size()));
    }

    const uint64 DataOffset = Reserve(static_cast<uint64>(Layout.RowPitch) * Layout.RowCount * Layout.Depth, TextureDataAlignment);
    for (uint32 Slice = 0; Slice < Layout.Depth; ++Slice)
    {
        for (uint32 Row = 0; Row < Layout.RowCount; ++Row)
        {
            const uint64 SourceRow = static_cast<uint64>(Slice) * Layout.RowCount + Row;
            std::memcpy(PendingData.data() + DataOffset + SourceRow * Layout.RowPitch, Data.data() + SourceRow * Layout.RowSize, Layout.RowSize);
        }
    }

    PendingTextureUploads.push_back({.Destination = Destination, .Subresource = Subresource, .DataOffset = DataOffset});
    return RESULT_OK_EMPTY();
}

void FUploadManager::Flush(IGfxCommandList& CommandList)
{
    if (!HasPendingUploads())
    {
        return;
    }

    const FGfxTransientAllocation Staging = CommandList.AllocateTransient(PendingData.size(), TextureDataAlignment);
    if (!Staging.IsValid())
    {
        RAVEN_LOG(LogRenderer, Error, "Upload: failed to allocate {} bytes of staging memory, {} uploads dropped",
                  PendingData.size(), PendingBufferUploads.size() + PendingTextureUploads.size());
        PendingData.clear();
        PendingBufferUploads.clear();
        PendingTextureUploads.clear();
        return;
    }
    std::memcpy(Staging.CpuData.data(), PendingData.data(), PendingData.size());

    CommandList.BeginEvent("Uploads");

    for (const FPendingBufferUpload& Upload : PendingBufferUploads)
    {
        CommandList.CopyBuffer(Upload.Destination, Upload.DestinationOffset, Staging.GpuRange.BufferHandle,
                               Staging.GpuRange.Offset + Upload.DataOffset, Upload.Size);
    }

    TextureBarriers.clear();
    for (const FPendingTextureUpload& Upload : PendingTextureUploads)
    {
        if (std::ranges::find(TextureBarriers, Upload.Destination, &FGfxTextureBarrier::TextureHandle) != TextureBarriers.end())
        {
            continue;
        }
        TextureBarriers.push_back({
            .TextureHandle = Upload.Destination,
            .SyncBefore = EGfxBarrierSync::None,
            .SyncAfter = EGfxBarrierSync::Copy,
            .AccessBefore = EGfxBarrierAccess::NoAccess,
            .AccessAfter = EGfxBarrierAccess::CopyDestination,
            .LayoutBefore = EGfxTextureLayout::Undefined,
            .LayoutAfter = EGfxTextureLayout::CopyDestination,
        });
    }
    if (!TextureBarriers.empty())
    {
        CommandList.Barrier({}, {}, TextureBarriers);
    }

    for (const FPendingTextureUpload& Upload : PendingTextureUploads)
    {
        CommandList.CopyBufferToTexture(Upload.Destination, Upload.Subresource, Staging.GpuRange.BufferHandle, Staging.GpuRange.Offset + Upload.DataOffset);
    }

    for (FGfxTextureBarrier& Barrier : TextureBarriers)
    {
        Barrier.SyncBefore = EGfxBarrierSync::Copy;
        Barrier.SyncAfter = EGfxBarrierSync::AllShading;
        Barrier.AccessBefore = EGfxBarrierAccess::CopyDestination;
        Barrier.AccessAfter = EGfxBarrierAccess::ShaderResource;
        Barrier.LayoutBefore = EGfxTextureLayout::CopyDestination;
        Barrier.LayoutAfter = EGfxTextureLayout::ShaderResource;
    }
    constexpr FGfxGlobalBarrier AfterBufferCopies = {
        .SyncBefore = EGfxBarrierSync::Copy,
        .SyncAfter = EGfxBarrierSync::AllShading | EGfxBarrierSync::IndexInput,
        .AccessBefore = EGfxBarrierAccess::CopyDestination,
        .AccessAfter = EGfxBarrierAccess::ShaderResource | EGfxBarrierAccess::IndexBuffer,
    };
    const bool8 bHasBufferCopies = !PendingBufferUploads.empty();
    CommandList.Barrier(TSpan<const FGfxGlobalBarrier>(&AfterBufferCopies, bHasBufferCopies ? 1 : 0), {}, TextureBarriers);
    CommandList.EndEvent();

    RAVEN_LOG(LogRenderer, Verbose, "Upload: {} buffer copies, {} texture copies, {} bytes",
              PendingBufferUploads.size(), PendingTextureUploads.size(), PendingData.size());
    PendingData.clear();
    PendingBufferUploads.clear();
    PendingTextureUploads.clear();
}
