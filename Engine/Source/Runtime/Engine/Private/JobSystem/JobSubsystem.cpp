// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "JobSystem/JobSubsystem.hpp"

#include <functional>
#include <iostream>
#include <thread>

#include "Profiling/Profiling.hpp"

#include "Threading/AtomicWait.hpp"

RAVEN_DEFINE_PROFILER_ZONE(JobExecute, Profiler::Colors::Orange);

struct FFiberTransit
{
    FJobFiberSlot* FiberSlot = nullptr;
    FJobCounter* Counter = nullptr;
    int64 TargetValue = 0;
};

struct FThreadData
{
    uint32 WorkerIndex = 0;
    std::function<void(uint32)> WorkerThreadMain;
};

struct FWorkerTls
{
    uint32 WorkerIndex = 0;
    FFiber* ThreadFiber = nullptr;
    FJobFiberSlot* CurrentFiberSlot = nullptr;
    FFiberTransit Transit = {};
};

static FWorkerTls& GetWorkerTls()
{
    thread_local FWorkerTls WorkerTls;
    return WorkerTls;
}

void FJob::Execute()
{
    if (Function)
    {
        RAVEN_PROFILE_ZONE(JobExecuteZone);
        Function(Payload);
    }
}

int64 FJobCounter::GetValue() const
{
    return Value.load(std::memory_order_acquire);
}

TResult<void> FJobSubsystem::Initialize()
{
    WorkerCount = std::thread::hardware_concurrency() - 1;
    FiberCount = 128; // TODO: Make configurable

    Fibers = static_cast<FJobFiberSlot*>(Memory::Allocate(sizeof(FJobFiberSlot) * FiberCount, alignof(FJobFiberSlot)));
    for (uint32 Index = 0; Index < FiberCount; ++Index)
    {
        std::construct_at(Fibers + Index);
    }
    FiberFreeList.Initialize(FiberCount);

    for (uint32 Index = 0; Index < FiberCount; ++Index)
    {
        FJobFiberSlot& Slot = Fibers[Index];
        Slot.Owner = this;
        Slot.Index = Index;
        std::snprintf(Slot.Name, sizeof(Slot.Name), "Fiber%03u", Index);

        const FPlatformFiberDesc PlatformFiberDesc = {
            .EntryFunction = &FiberEntry,
            .Data = &Slot,
            .CommitSize = 0,
            .ReserveSize = 0,
            .Name = Slot.Name,
        };
        Slot.Fiber.Initialize(PlatformFiberDesc);
        FiberFreeList.Push(Index);
    }

    WorkerThreads.reserve(WorkerCount);
    for (uint32 Index = 0; Index < WorkerCount; ++Index)
    {
        char ThreadName[32];
        std::snprintf(ThreadName, sizeof(ThreadName), "Worker%02u", Index);

        FThreadData* ThreadData = Memory::New<FThreadData>();
        ThreadData->WorkerThreadMain = [&](const uint32 WorkerIndex)
        {
            WorkerThreadMain(WorkerIndex);
        };
        ThreadData->WorkerIndex = Index;
        const FThreadDesc ThreadDesc = {
            .EntryFunction = &ThreadEntry,
            .Data = ThreadData,
            .Name = ThreadName,
            .Priority = EThreadPriority::High,
        };
        WorkerThreads.emplace_back(ThreadDesc);
    }
    return {};
}

void FJobSubsystem::Shutdown()
{
    if (!Fibers)
    {
        return;
    }
    bShouldShutdown.store(true, std::memory_order_release);
    WakeWorkers(~0u);
    for (FThread& WorkerThread : WorkerThreads)
    {
        WorkerThread.Join();
    }
    WorkerThreads.clear();
    for (uint32 Index = 0; Index < FiberCount; ++Index)
    {
        std::destroy_at(Fibers + FiberCount - 1 - Index);
    }
    Memory::Free(Fibers);
    Fibers = nullptr;
}

void FJobSubsystem::WaitForCounter(FJobCounter& Counter, const int64 TargetValue)
{
    if (Counter.Value.load(std::memory_order_acquire) <= TargetValue)
    {
        return;
    }

    FWorkerTls& WorkerTls = GetWorkerTls();
    if (!WorkerTls.CurrentFiberSlot)
    {
        WaitForCounterBlocking(Counter, TargetValue);
        return;
    }

    FJobFiberSlot* Self = WorkerTls.CurrentFiberSlot;
    const FJobFiberSlot* Next = AcquireNextFiberSlot();
    WorkerTls.Transit = {Self, &Counter, TargetValue};
    FFiber::Switch(Self->Fiber, Next->Fiber);

    // This happens after the next fiber returns
    FWorkerTls& NewWorkerTls = GetWorkerTls();
    NewWorkerTls.CurrentFiberSlot = Self;
    FinishTransit(NewWorkerTls);
}

void FJobSubsystem::WaitForCounterBlocking(FJobCounter& Counter, const int64 TargetValue)
{
    for (uint32 Spin = 0; Spin < BlockingSpinCount; ++Spin)
    {
        if (Counter.Value.load(std::memory_order_acquire) <= TargetValue)
        {
            return;
        }
        _mm_pause();
    }
    Counter.bHasBlockingWaiters.store(true, std::memory_order_relaxed);
    for (;;)
    {
        const uint32 Epoch = Counter.BlockingEpoch.load(std::memory_order_acquire);
        if (Counter.Value.load(std::memory_order_acquire) <= TargetValue)
        {
            return;
        }
        AtomicWait::Wait(Counter.BlockingEpoch, Epoch);
    }
}

void FJobSubsystem::FiberEntry(void* Data)
{
    FJobFiberSlot& Slot = *static_cast<FJobFiberSlot*>(Data);
    FJobSubsystem& System = *Slot.Owner;

    for (;;)
    {
        FWorkerTls& WorkerTls = GetWorkerTls();
        WorkerTls.CurrentFiberSlot = &Slot;
        System.FinishTransit(WorkerTls);

        if (System.bShouldShutdown.load(std::memory_order_acquire))
        {
            WorkerTls.Transit = {&Slot, nullptr, 0};
            FFiber::Switch(Slot.Fiber, *WorkerTls.ThreadFiber);
            continue; // Will not be reached
        }

        const uint32 Epoch = System.WorkEpoch.load(std::memory_order_acquire);
        FJobFiberSlot* ReadySlot = nullptr;
        if (System.ResumableFiberSlots.TryDequeue(ReadySlot))
        {
            WorkerTls.Transit = {&Slot, nullptr, 0};
            FFiber::Switch(Slot.Fiber, ReadySlot->Fiber);
            continue; // Will be resumed
        }

        FJob Job;
        if (System.TryDequeueJob(Job))
        {
            Job.Execute();
            if (Job.Counter)
            {
                System.DecrementCounter(*Job.Counter);
            }
            continue;
        }

        System.WaitForWork(Epoch);
    }
}

void FJobSubsystem::ThreadEntry(void* Data)
{
    const FThreadData ThreadData = *static_cast<FThreadData*>(Data);
    Memory::Delete(static_cast<FThreadData*>(Data));
    ThreadData.WorkerThreadMain(ThreadData.WorkerIndex);
}

void FJobSubsystem::EnqueueJob(const FJob& Job)
{
    RAVEN_PROFILE_FUNCTION();
    EnqueueJobNoWake(Job);
    WakeWorkers(1);
}

void FJobSubsystem::EnqueueJobNoWake(const FJob& Job)
{
    RAVEN_PROFILE_FUNCTION();
    TMpmcQueue<FJob, JobQueueCapacity>& Queue = JobQueues[std::to_underlying(Job.Priority)];
    while (!Queue.TryEnqueue(Job))
    {
        _mm_pause();
    }
}

bool8 FJobSubsystem::TryDequeueJob(FJob& OutJob)
{
    for (TMpmcQueue<FJob, JobQueueCapacity>& JobQueue : JobQueues)
    {
        if (JobQueue.TryDequeue(OutJob))
        {
            return true;
        }
    }
    return false;
}

FJobFiberSlot* FJobSubsystem::AcquireNextFiberSlot()
{
    for (uint64 Spin = 0;; ++Spin)
    {
        FJobFiberSlot* ResumableSlot = nullptr;
        if (ResumableFiberSlots.TryDequeue(ResumableSlot))
        {
            return ResumableSlot;
        }

        uint32 Index = 0;
        if (FiberFreeList.TryPop(Index))
        {
            return &Fibers[Index];
        }
        _mm_pause();
    }
}

void FJobSubsystem::FinishTransit(FWorkerTls& WorkerTls)
{
    const FFiberTransit Transit = WorkerTls.Transit;
    WorkerTls.Transit = {};
    if (!Transit.FiberSlot)
    {
        return;
    }
    if (!Transit.Counter)
    {
        FiberFreeList.Push(Transit.FiberSlot->Index);
        return;
    }

    FJobCounter& Counter = *Transit.Counter;
    Counter.SpinLock.Lock();
    if (Counter.Value.load(std::memory_order_acquire) <= Transit.TargetValue)
    {
        Counter.SpinLock.Unlock();
        while (!ResumableFiberSlots.TryEnqueue(Transit.FiberSlot))
        {
            _mm_pause();
        }
        WakeWorkers(1);
    }
    else
    {
        Counter.Waiters[Counter.WaiterCount++] = {.FiberSlot = Transit.FiberSlot, .TargetValue = Transit.TargetValue};
        Counter.SpinLock.Unlock();
    }
}

void FJobSubsystem::DecrementCounter(FJobCounter& Counter)
{
    const int64 NewValue = Counter.Value.fetch_sub(1, std::memory_order_acq_rel) - 1;
    FJobFiberSlot* Woken[FJobCounter::MaxWaiters] = {};
    uint32 NumWoken = 0;
    FJob Fired[FJobCounter::MaxContinuations] = {};
    uint32 NumFired = 0;

    Counter.SpinLock.Lock();
    for (size64 Index = 0; Index < Counter.WaiterCount;)
    {
        if (NewValue <= Counter.Waiters[Index].TargetValue)
        {
            Woken[NumWoken++] = Counter.Waiters[Index].FiberSlot;
            Counter.Waiters[Index] = Counter.Waiters[--Counter.WaiterCount];
        }
        else
        {
            ++Index;
        }
    }

    for (size64 Index = 0; Index < Counter.ContinuationCount;)
    {
        if (NewValue <= Counter.Continuations[Index].TargetValue)
        {
            Fired[NumFired++] = Counter.Continuations[Index].Job;
            Counter.Continuations[Index] = Counter.Continuations[--Counter.ContinuationCount];
        }
        else
        {
            ++Index;
        }
    }
    Counter.SpinLock.Unlock();

    for (size64 Index = 0; Index < NumWoken; ++Index)
    {
        while (!ResumableFiberSlots.TryEnqueue(Woken[Index]))
        {
            _mm_pause();
        }
    }
    for (size64 Index = 0; Index < NumFired; ++Index)
    {
        EnqueueJob(Fired[Index]);
    }
    if (NumWoken > 0)
    {
        WakeWorkers(NumWoken);
    }

    if (Counter.bHasBlockingWaiters.load(std::memory_order_relaxed))
    {
        Counter.BlockingEpoch.fetch_add(1, std::memory_order_release);
        AtomicWait::WakeAll(Counter.BlockingEpoch);
    }
}

void FJobSubsystem::WakeWorkers(const uint32 Count)
{
    WorkEpoch.fetch_add(1, std::memory_order_release);
    if (NumSleepingWorkers.load(std::memory_order_relaxed) == 0)
    {
        return;
    }
    if (Count == 1)
    {
        AtomicWait::WakeOne(WorkEpoch);
    }
    else
    {
        AtomicWait::WakeAll(WorkEpoch);
    }
}

void FJobSubsystem::WaitForWork(const uint32 ObservedEpoch)
{
    for (uint32 Spin = 0; Spin < IdleSpinCount; ++Spin)
    {
        if (WorkEpoch.load(std::memory_order_acquire) != ObservedEpoch || bShouldShutdown.load(std::memory_order_relaxed))
        {
            return;
        }
        _mm_pause();
    }
    if (bShouldShutdown.load(std::memory_order_acquire))
    {
        return;
    }
    NumSleepingWorkers.fetch_add(1, std::memory_order_release);
    {
        RAVEN_PROFILE_ZONE_CUSTOM("WaitForWork", 0x1E1E1E);
        AtomicWait::Wait(WorkEpoch, ObservedEpoch);
    }
    NumSleepingWorkers.fetch_sub(1, std::memory_order_release);
}

void FJobSubsystem::WorkerThreadMain(const uint32 WorkerIndex)
{
    FWorkerTls& WorkerTls = GetWorkerTls();
    WorkerTls.WorkerIndex = WorkerIndex;

    FThreadFiberScope ThreadFiberScope;
    WorkerTls.ThreadFiber = &ThreadFiberScope.GetFiber();

    const FJobFiberSlot* FirstSlot = AcquireNextFiberSlot();
    FFiber::Switch(ThreadFiberScope.GetFiber(), FirstSlot->Fiber);

    WorkerTls.CurrentFiberSlot = nullptr;
    FinishTransit(WorkerTls);
}
