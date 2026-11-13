// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12CommandListPool.hpp"

#include <print>

#include "Graphics/D3D12/D3D12GfxCommandList.hpp"
#include "Graphics/D3D12/D3D12GfxQueue.hpp"

void FD3D12CommandListPool::Initialize(FD3D12GfxDevice& InDevice, ID3D12Device4* InNativeDevice, FD3D12GfxQueue& InQueue)
{
    Device = &InDevice;
    NativeDevice = InNativeDevice;
    Queue = &InQueue;
}

void FD3D12CommandListPool::Deinitialize()
{
    std::scoped_lock Lock(Mutex);
    RetiredAllocators.clear();
    FreeCommandLists.clear();
    CommandLists.clear();
    AllocatorCount = 0;
}

FD3D12GfxCommandList* FD3D12CommandListPool::Acquire()
{
    TComPtr<ID3D12CommandAllocator> Allocator;
    FD3D12GfxCommandList* CommandList = nullptr;
    {
        std::scoped_lock Lock(Mutex);
        if (!RetiredAllocators.empty() && Queue->IsComplete(RetiredAllocators.front().FenceValue))
        {
            Allocator = std::move(RetiredAllocators.front().Allocator);
            RetiredAllocators.pop_front();
        }
        if (!FreeCommandLists.empty())
        {
            CommandList = FreeCommandLists.back();
            FreeCommandLists.pop_back();
        }
    }

    if (Allocator)
    {
        if (FAILED(Allocator->Reset()))
        {
            std::println("Failed to reset command allocator");
            Allocator = nullptr;
        }
    }
    if (!Allocator)
    {
        Allocator = CreateAllocator();
    }
    if (!CommandList)
    {
        CommandList = CreateCommandList();
    }
    if (!Allocator || !CommandList)
    {
        std::scoped_lock Lock(Mutex);
        if (CommandList)
        {
            FreeCommandLists.push_back(CommandList);
        }
        return nullptr;
    }

    CommandList->Begin(std::move(Allocator));
    return CommandList;
}

void FD3D12CommandListPool::Recycle(const TSpan<FD3D12GfxCommandList* const> InCommandLists, const uint64 FenceValue)
{
    std::scoped_lock Lock(Mutex);
    for (FD3D12GfxCommandList* CommandList : InCommandLists)
    {
        RetiredAllocators.push_back(FRetiredAllocator{FenceValue, CommandList->TakeAllocator()});
        FreeCommandLists.push_back(CommandList);
    }
}

TComPtr<ID3D12CommandAllocator> FD3D12CommandListPool::CreateAllocator()
{
    TComPtr<ID3D12CommandAllocator> Allocator;
    if (FAILED(NativeDevice->CreateCommandAllocator(ToD3D12CommandListType(Queue->GetType()), IID_PPV_ARGS(&Allocator))))
    {
        std::println("Failed to create command allocator");
        return nullptr;
    }

    uint32 Index;
    {
        std::scoped_lock Lock(Mutex);
        Index = AllocatorCount++;
    }
    const FWideString Name = String::Format(L"{}CommandAllocator{}", ToD3D12QueueName(Queue->GetType()), Index);
    Allocator->SetName(Name.c_str());
    return Allocator;
}

FD3D12GfxCommandList* FD3D12CommandListPool::CreateCommandList()
{
    TComPtr<ID3D12GraphicsCommandList10> NativeList;
    if (FAILED(NativeDevice->CreateCommandList1(0, ToD3D12CommandListType(Queue->GetType()), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&NativeList))))
    {
        std::println("Failed to create command list");
        return nullptr;
    }

    std::scoped_lock Lock(Mutex);
    const FWideString Name = String::Format(L"{}CommandList{}", ToD3D12QueueName(Queue->GetType()), CommandLists.size());
    NativeList->SetName(Name.c_str());
    CommandLists.push_back(MakeUnique<FD3D12GfxCommandList>(*Device, Queue->GetType(), std::move(NativeList)));
    return CommandLists.back().get();
}
