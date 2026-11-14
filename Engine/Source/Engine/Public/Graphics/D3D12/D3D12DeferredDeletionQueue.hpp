// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <functional>
#include <mutex>

#include "D3D12GfxSync.hpp"
#include "D3D12Include.hpp"
#include "Core/CoreTypes.hpp"
#include "Core/Memory/Containers/Queue.hpp"
#include "Core/Memory/Containers/Vector.hpp"

class FD3D12GfxQueue;

class RAVEN_API FD3D12DeferredDeletionQueue
{
public:
    using FReleaseCallback = std::move_only_function<void()>;

public:
    void Initialize(FD3D12GfxQueue* InQueues);

    void Enqueue(TComPtr<IUnknown> Object, FReleaseCallback OnRelease = {});
    void Retire(const FD3D12QueueFenceValues& FenceValues);

    void Process();
    void Flush();

private:
    struct FEntry
    {
        TComPtr<IUnknown> Object;
        FReleaseCallback OnRelease;
    };

    struct FBatch
    {
        FD3D12QueueFenceValues FenceValues;
        TVector<FEntry> Entries;
    };

private:
    [[nodiscard]] bool8 IsRetired(const FD3D12QueueFenceValues& FenceValues) const;

private:
    static void ReleaseEntries(TVector<FEntry>& Entries);

private:
    FD3D12GfxQueue* Queues = nullptr;

    std::mutex Mutex;
    TVector<FEntry> Pending;
    TDeque<FBatch> Batches;
    TVector<FBatch> ReadyScratch;
};
