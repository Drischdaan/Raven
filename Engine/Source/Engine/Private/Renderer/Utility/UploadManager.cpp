// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Utility/UploadManager.hpp"

#include <algorithm>

#include "Graphics/GfxBarriers.hpp"
#include "Graphics/GfxCommandList.hpp"
#include "Renderer/RendererLog.hpp"

void FUploadManager::UploadBuffer(const FGfxBufferHandle Destination, const uint64 DestinationOffset, const TSpan<const byte8> Data)
{
    if (Data.empty())
    {
        return;
    }
    const uint64 DataOffset = (PendingData.size() + 15) & ~15ull;
    PendingData.resize(DataOffset + Data.size());
    std::memcpy(PendingData.data() + DataOffset, Data.data(), Data.size());
    PendingUploads.push_back({.Destination = Destination, .DestinationOffset = DestinationOffset, .DataOffset = DataOffset, .Size = Data.size()});
}

void FUploadManager::Flush(IGfxCommandList& CommandList)
{
    if (PendingUploads.empty())
    {
        return;
    }

    const FGfxTransientAllocation Staging = CommandList.AllocateTransient(PendingData.size(), 16);
    if (!Staging.IsValid())
    {
        RAVEN_LOG(LogRenderer, Error, "Upload: failed to allocate {} bytes of staging memory, {} uploads dropped", PendingData.size(), PendingUploads.size());
        PendingData.clear();
        PendingUploads.clear();
        return;
    }
    std::memcpy(Staging.CpuData.data(), PendingData.data(), PendingData.size());

    CommandList.BeginEvent("Uploads");
    for (const FPendingUpload& Upload : PendingUploads)
    {
        CommandList.CopyBuffer(Upload.Destination, Upload.DestinationOffset, Staging.GpuRange.BufferHandle,
                               Staging.GpuRange.Offset + Upload.DataOffset, Upload.Size);
    }

    constexpr FGfxGlobalBarrier AfterCopies = {
        .SyncBefore = EGfxBarrierSync::Copy,
        .SyncAfter = EGfxBarrierSync::AllShading | EGfxBarrierSync::IndexInput,
        .AccessBefore = EGfxBarrierAccess::CopyDestination,
        .AccessAfter = EGfxBarrierAccess::ShaderResource | EGfxBarrierAccess::IndexBuffer,
    };
    CommandList.Barrier(TSpan<const FGfxGlobalBarrier>(&AfterCopies, 1), {}, {});
    CommandList.EndEvent();

    RAVEN_LOG(LogRenderer, Verbose, "Upload: {} copies, {} bytes", PendingUploads.size(), PendingData.size());
    PendingData.clear();
    PendingUploads.clear();
}
