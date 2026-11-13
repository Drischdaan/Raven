// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <mutex>

#include "D3D12GfxCommandList.hpp"
#include "D3D12Include.hpp"
#include "Core/Memory/SmartPointers.hpp"
#include "Core/Memory/Containers/Queue.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Memory/Containers/Vector.hpp"

class FD3D12GfxQueue;
class FD3D12GfxDevice;

class RAVEN_API FD3D12CommandListPool
{
public:
    void Initialize(FD3D12GfxDevice& InDevice, ID3D12Device4* InNativeDevice, FD3D12GfxQueue& InQueue);
    void Deinitialize();

    [[nodiscard]] FD3D12GfxCommandList* Acquire();
    void Recycle(TSpan<FD3D12GfxCommandList* const> InCommandLists, uint64 FenceValue);

private:
    [[nodiscard]] TComPtr<ID3D12CommandAllocator> CreateAllocator();
    [[nodiscard]] FD3D12GfxCommandList* CreateCommandList();

private:
    struct FRetiredAllocator
    {
        uint64 FenceValue = 0;
        TComPtr<ID3D12CommandAllocator> Allocator;
    };

private:
    FD3D12GfxDevice* Device = nullptr;
    ID3D12Device4* NativeDevice = nullptr;
    FD3D12GfxQueue* Queue = nullptr;

    std::mutex Mutex;
    TVector<TUniquePtr<FD3D12GfxCommandList>> CommandLists;
    TVector<FD3D12GfxCommandList*> FreeCommandLists;
    TDeque<FRetiredAllocator> RetiredAllocators;
    uint32 AllocatorCount = 0;
};
