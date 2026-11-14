// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12DeferredDeletionQueue.hpp"

#include "Graphics/D3D12/D3D12GfxQueue.hpp"

void FD3D12DeferredDeletionQueue::Initialize(FD3D12GfxQueue* InQueues)
{
    Queues = InQueues;
}

void FD3D12DeferredDeletionQueue::Enqueue(TComPtr<IUnknown> Object, FReleaseCallback OnRelease)
{
    if (!Object && !OnRelease)
    {
        return;
    }
    std::scoped_lock Lock(Mutex);
    Pending.push_back(FEntry{std::move(Object), std::move(OnRelease)});
}

void FD3D12DeferredDeletionQueue::Retire(const FD3D12QueueFenceValues& FenceValues)
{
    std::scoped_lock Lock(Mutex);
    if (Pending.empty())
    {
        return;
    }
    FBatch Batch;
    Batch.FenceValues = FenceValues;
    Batch.Entries = std::move(Pending);
    Pending.clear();
    Batches.push_back(std::move(Batch));
}

void FD3D12DeferredDeletionQueue::Process()
{
    {
        std::scoped_lock Lock(Mutex);
        while (!Batches.empty() && IsRetired(Batches.front().FenceValues))
        {
            ReadyScratch.push_back(std::move(Batches.front()));
            Batches.pop_front();
        }
    }

    for (FBatch& Batch : ReadyScratch)
    {
        ReleaseEntries(Batch.Entries);
    }
    ReadyScratch.clear();
}

void FD3D12DeferredDeletionQueue::Flush()
{
    TVector<FEntry> PendingToRelease;
    TDeque<FBatch> BatchesToRelease;
    {
        std::scoped_lock Lock(Mutex);
        PendingToRelease = std::move(Pending);
        BatchesToRelease = std::move(Batches);
        Pending.clear();
        Batches.clear();
    }

    for (FBatch& Batch : BatchesToRelease)
    {
        ReleaseEntries(Batch.Entries);
    }
    ReleaseEntries(PendingToRelease);
}

bool8 FD3D12DeferredDeletionQueue::IsRetired(const FD3D12QueueFenceValues& FenceValues) const
{
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        if (!Queues[Index].IsComplete(FenceValues.Values[Index]))
        {
            return false;
        }
    }
    return true;
}

void FD3D12DeferredDeletionQueue::ReleaseEntries(TVector<FEntry>& Entries)
{
    for (FEntry& Entry : Entries)
    {
        Entry.Object.Reset();
        if (Entry.OnRelease)
        {
            Entry.OnRelease();
        }
    }
    Entries.clear();
}
