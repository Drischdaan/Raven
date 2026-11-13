// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <mutex>

#include "D3D12Include.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Utility/Result.hpp"
#include "Graphics/GfxTypes.hpp"

class FD3D12GfxDevice;

[[nodiscard]] D3D12_COMMAND_LIST_TYPE ToD3D12CommandListType(EGfxQueueType QueueType);

[[nodiscard]] const TWideChar* ToD3D12QueueName(EGfxQueueType QueueType);

class RAVEN_API FD3D12GfxQueue
{
public:
    FResult Initialize(ID3D12Device* InDevice, EGfxQueueType InQueueType);
    void Deinitialize();

    FGfxSyncPoint Submit(TSpan<ID3D12CommandList* const> CommandLists, TSpan<const FGfxSyncPoint> WaitFor, const FD3D12GfxDevice& OwningDevice);
    FGfxSyncPoint Signal();

    void GpuWait(ID3D12Fence* OtherFence, uint64 Value);

    [[nodiscard]] bool8 IsComplete(uint64 Value);
    void CpuWait(uint64 Value);
    void WaitForIdle();

    [[nodiscard]] ID3D12CommandQueue* GetNative() const;
    [[nodiscard]] ID3D12Fence* GetFence() const;
    [[nodiscard]] EGfxQueueType GetType() const;
    [[nodiscard]] uint64 GetLastSubmittedValue() const;

private:
    uint64 SignalLocked();

private:
    EGfxQueueType QueueType = EGfxQueueType::Graphics;
    TComPtr<ID3D12CommandQueue> CommandQueue;
    TComPtr<ID3D12Fence1> Fence;

    std::mutex SubmitMutex;
    std::atomic<uint64> NextValue = 1;
    std::atomic<uint64> LastCompletedValue = 0;
};
