// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "FiberFreeList.hpp"
#include "SpinLock.hpp"
#include "Subsystem.hpp"

#include "Memory/Containers/MpmcQueue.hpp"
#include "Memory/Containers/Vector.hpp"

#include "Profiling/Profiling.hpp"

#include "Threading/Fiber.hpp"
#include "Threading/Thread.hpp"

#include "Utility/Atomic.hpp"

class FJobCounter;
struct FWorkerTls;

enum class ENGINE_API EJobPriority : uint8
{
    High = 0,
    Low = 1,
    Normal = 2,
    Count,
};

inline static constexpr uint8 JobInlinePayloadSize = 48;

using FJobFunction = void(*)(void* Payload);

struct ENGINE_API FJob
{
    FJobFunction Function = nullptr;
    FJobCounter* Counter = nullptr;
    EJobPriority Priority = EJobPriority::Normal;
    alignas(16) uint8 Payload[JobInlinePayloadSize] = {};

public:
    void Execute();
};

struct ENGINE_API FJobFiberSlot
{
    FFiber Fiber;
    class FJobSubsystem* Owner = nullptr;
    uint32 Index = 0;
    TChar Name[32] = "UnnamedFiberSlot";
};

class ENGINE_API FJobCounter
{
private:
    struct FWaiter
    {
        FJobFiberSlot* FiberSlot = nullptr;
        int64 TargetValue = 0;
    };

    struct FContinuation
    {
        FJob Job;
        int64 TargetValue = 0;
    };

public:
    FJobCounter() = default;
    FJobCounter(const FJobCounter&) = delete;
    FJobCounter(FJobCounter&& Other) noexcept = delete;
    ~FJobCounter() = default;

    FJobCounter& operator=(const FJobCounter&) = delete;
    FJobCounter& operator=(FJobCounter&& Other) noexcept = delete;

public:
    [[nodiscard]] int64 GetValue() const;

private:
    static constexpr size64 MaxWaiters = 8;
    static constexpr size64 MaxContinuations = 4;

private:
    TAtomic<int64> Value = 0;
    TAtomic<uint32> BlockingEpoch = 0;
    TAtomic<bool8> bHasBlockingWaiters = false;

    FSpinLock SpinLock;
    FWaiter Waiters[MaxWaiters] = {};
    size64 WaiterCount = 0;

    FContinuation Continuations[MaxContinuations] = {};
    size64 ContinuationCount = 0;

    friend class FJobSubsystem;
};

// TODO: Add more assertions. There are a lot of places where the application can crash
class ENGINE_API FJobSubsystem : public ISubsystem
{
public:
    [[nodiscard]] TResult<void> Initialize() override;
    void Shutdown() override;

public:
    void WaitForCounter(FJobCounter& Counter, int64 TargetValue = 0);
    void WaitForCounterBlocking(FJobCounter& Counter, int64 TargetValue = 0);

private:
    static void FiberEntry(void* Data);
    static void ThreadEntry(void* Data);

private:
    void EnqueueJob(const FJob& Job);
    void EnqueueJobNoWake(const FJob& Job);
    [[nodiscard]] bool8 TryDequeueJob(FJob& OutJob);

    FJobFiberSlot* AcquireNextFiberSlot();

    void FinishTransit(FWorkerTls& WorkerTls);
    void DecrementCounter(FJobCounter& Counter);

    void WakeWorkers(uint32 Count);
    void WaitForWork(uint32 ObservedEpoch);
    void WorkerThreadMain(uint32 WorkerIndex);

public:
    template <typename TBody>
    void ParallelFor(const size64 Count, TBody&& Body, size64 BatchSize = 0, const EJobPriority Priority = EJobPriority::Normal)
    {
        RAVEN_PROFILE_FUNCTION();
        if (Count == 0)
        {
            return;
        }

        if (BatchSize == 0)
        {
            const size64 TargetBatches = static_cast<size64>(WorkerCount) * 4;
            BatchSize = (Count + TargetBatches - 1) / TargetBatches;

            constexpr size64 MinItemsPerBatch = 64;
            if (BatchSize < MinItemsPerBatch)
            {
                BatchSize = MinItemsPerBatch;
            }
        }

        const size64 NumBatches = (Count + BatchSize - 1) / BatchSize;

        FJobCounter Counter;
        Counter.Value.fetch_add(static_cast<int64>(NumBatches), std::memory_order_release);
        for (size64 Begin = 0; Begin < Count; Begin += BatchSize)
        {
            const size64 End = (Begin + BatchSize < Count) ? (Begin + BatchSize) : Count;
            EnqueueJobNoWake(MakeJob([&Body, Begin, End]
            {
                if constexpr (std::invocable<TBody&, size64, size64>)
                {
                    Body(Begin, End);
                }
                else
                {
                    for (size64 Index = Begin; Index < End; ++Index)
                    {
                        Body(Index);
                    }
                }
            }, &Counter, Priority));
        }
        WakeWorkers(static_cast<uint32>(NumBatches < WorkerCount ? NumBatches : WorkerCount));
        WaitForCounter(Counter, 0);
    }

    template <typename TCallable>
    void Kick(TCallable&& Callable, FJobCounter* Counter = nullptr,
              EJobPriority Priority = EJobPriority::Normal)
    {
        RAVEN_PROFILE_FUNCTION();
        const FJob Job = MakeJob(std::forward<TCallable>(Callable), Counter, Priority);
        if (Counter)
            Counter->Value.fetch_add(1, std::memory_order_release);
        EnqueueJob(Job);
    }

    template <typename TCallable>
    void KickAfter(FJobCounter& After, const int64 Target, TCallable&& Callable,
                   FJobCounter* Signal = nullptr, EJobPriority Priority = EJobPriority::Normal)
    {
        RAVEN_PROFILE_FUNCTION();
        const FJob Job = MakeJob(std::forward<TCallable>(Callable), Signal, Priority);
        if (Signal)
            Signal->Value.fetch_add(1, std::memory_order_release);

        After.SpinLock.Lock();
        if (After.Value.load(std::memory_order_acquire) <= Target)
        {
            After.SpinLock.Unlock();
            EnqueueJob(Job);
            return;
        }
        RAVEN_CORE_ASSERT(After.ContinuationCount < FJobCounter::MaxContinuations,
                          "FJobCounter continuation slots exhausted");
        After.Continuations[After.ContinuationCount++] = {Job, Target};
        After.SpinLock.Unlock();
    }

private:
    template <typename TCallable>
    static FJob MakeJob(TCallable&& Callable, FJobCounter* Counter, const EJobPriority Priority)
    {
        using FDecayed = std::decay_t<TCallable>;
        static_assert(std::invocable<FDecayed&>, "Job callable must be invocable with no arguments");

        FJob Job;
        Job.Counter = Counter;
        Job.Priority = Priority;

        if constexpr (sizeof(FDecayed) <= JobInlinePayloadSize
            && alignof(FDecayed) <= 16
            && std::is_trivially_copyable_v<FDecayed>
            && std::is_trivially_destructible_v<FDecayed>)
        {
            ::new(static_cast<void*>(Job.Payload)) FDecayed(std::forward<TCallable>(Callable));
            Job.Function = [](void* Payload)
            {
                (*std::launder(static_cast<FDecayed*>(Payload)))();
            };
        }
        else
        {
            // Heap fallback (mimalloc). If hot, capture by pointer/span instead of by value.
            TCallable* Boxed = Memory::New<FDecayed>(std::forward<TCallable>(Callable));
            RAVEN_PROFILE_ALLOCATION(Boxed, sizeof(FDecayed));
            std::memcpy(Job.Payload, &Boxed, sizeof(Boxed));
            Job.Function = [](const void* Payload)
            {
                FDecayed* Pointer;
                Memory::Copy(&Pointer, Payload, sizeof(Pointer));
                (*Pointer)();
                RAVEN_PROFILE_FREE(Pointer);
                Memory::Delete(Pointer);
            };
        }
        return Job;
    }

private:
    static constexpr size64 JobQueueCapacity = 4096;
    static constexpr size64 ResumableCapacity = 512;
    static constexpr size64 IdleSpinCount = 1000;
    static constexpr size64 BlockingSpinCount = 4096;

private:
    TMpmcQueue<FJob, JobQueueCapacity> JobQueues[static_cast<size64>(EJobPriority::Count)];
    TMpmcQueue<FJobFiberSlot*, ResumableCapacity> ResumableFiberSlots;
    FFiberFreeList FiberFreeList;
    FJobFiberSlot* Fibers = nullptr;
    TVector<FThread> WorkerThreads;

    TAtomic<uint32> NumSleepingWorkers = 0;
    alignas(64) TAtomic<uint32> WorkEpoch = 0;
    TAtomic<bool8> bShouldShutdown = false;
    uint32 WorkerCount = 0;
    uint32 FiberCount = 0;
};
