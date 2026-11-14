// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12GfxQueue.hpp"

#include "Graphics/D3D12/D3D12GfxDevice.hpp"

D3D12_COMMAND_LIST_TYPE ToD3D12CommandListType(const EGfxQueueType QueueType)
{
    switch (QueueType)
    {
    case EGfxQueueType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case EGfxQueueType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case EGfxQueueType::Copy: return D3D12_COMMAND_LIST_TYPE_COPY;
    case EGfxQueueType::Count:
    default:
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
}

const TWideChar* ToD3D12QueueName(const EGfxQueueType QueueType)
{
    switch (QueueType)
    {
    case EGfxQueueType::Graphics: return L"GraphicsQueue";
    case EGfxQueueType::Compute: return L"ComputeQueue";
    case EGfxQueueType::Copy: return L"CopyQueue";
    case EGfxQueueType::Count:
    default:
        return L"UnknownQueue";
    }
}

FResult FD3D12GfxQueue::Initialize(ID3D12Device* InDevice, const EGfxQueueType InQueueType)
{
    QueueType = InQueueType;

    D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
    QueueDesc.Type = ToD3D12CommandListType(QueueType);
    QueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
    QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    RESULT_TRY_PLATFORM_CALL(InDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue)), "Failed to create command queue")
    RESULT_TRY_PLATFORM_CALL(InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)), "Failed to create fence")

    CommandQueue->SetName(ToD3D12QueueName(QueueType));
    Fence->SetName(ToD3D12QueueName(QueueType));

    NextValue = 1;
    LastCompletedValue = 0;
    return RESULT_OK_EMPTY();
}

void FD3D12GfxQueue::Deinitialize()
{
    if (CommandQueue)
    {
        WaitForIdle();
    }
    Fence.Reset();
    CommandQueue.Reset();
}

FGfxSyncPoint FD3D12GfxQueue::Submit(const TSpan<ID3D12CommandList* const> CommandLists, const TSpan<const FGfxSyncPoint> WaitFor, const FD3D12GfxDevice& OwningDevice)
{
    std::scoped_lock Lock(SubmitMutex);
    for (const FGfxSyncPoint& SyncPoint : WaitFor)
    {
        if (SyncPoint.QueueType == QueueType)
        {
            continue;
        }
        FD3D12GfxQueue& OtherQueue = OwningDevice.GetQueue(SyncPoint.QueueType);
        if (!OtherQueue.IsComplete(SyncPoint.Value))
        {
            CommandQueue->Wait(OtherQueue.GetFence(), SyncPoint.Value);
        }
    }
    if (!CommandLists.empty())
    {
        CommandQueue->ExecuteCommandLists(static_cast<uint32>(CommandLists.size()), CommandLists.data());
    }
    return FGfxSyncPoint{QueueType, SignalLocked()};
}

FGfxSyncPoint FD3D12GfxQueue::Signal()
{
    std::scoped_lock Lock(SubmitMutex);
    return FGfxSyncPoint{QueueType, SignalLocked()};
}

void FD3D12GfxQueue::GpuWait(ID3D12Fence* OtherFence, const uint64 Value)
{
    std::scoped_lock Lock(SubmitMutex);
    CommandQueue->Wait(OtherFence, Value);
}

bool8 FD3D12GfxQueue::IsComplete(const uint64 Value)
{
    if (Value <= LastCompletedValue.load(std::memory_order_acquire))
    {
        return true;
    }

    const uint64 Completed = Fence->GetCompletedValue();
    if (Completed == UINT64_MAX)
    {
        // This is a device removed situation
        // TODO: Handle properly
        return true;
    }

    uint64 Cached = LastCompletedValue.load(std::memory_order_relaxed);
    while (Completed > Cached && !LastCompletedValue.compare_exchange_weak(Cached, Completed, std::memory_order_acq_rel))
    {
    }
    return Value <= Completed;
}

void FD3D12GfxQueue::CpuWait(const uint64 Value)
{
    if (IsComplete(Value))
    {
        return;
    }
    Fence->SetEventOnCompletion(Value, nullptr);
    (void)IsComplete(Value); // refresh cache
}

void FD3D12GfxQueue::WaitForIdle()
{
    CpuWait(Signal().Value);
}

ID3D12CommandQueue* FD3D12GfxQueue::GetNative() const
{
    return CommandQueue.Get();
}

ID3D12Fence* FD3D12GfxQueue::GetFence() const
{
    return Fence.Get();
}

EGfxQueueType FD3D12GfxQueue::GetType() const
{
    return QueueType;
}

uint64 FD3D12GfxQueue::GetLastSubmittedValue() const
{
    return NextValue.load(std::memory_order_acquire) - 1;
}

uint64 FD3D12GfxQueue::SignalLocked()
{
    const uint64 Value = NextValue.fetch_add(1, std::memory_order_acq_rel);
    CommandQueue->Signal(Fence.Get(), Value);
    return Value;
}
