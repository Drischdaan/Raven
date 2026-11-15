// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12DescriptorHeap.hpp"

bool8 FD3D12Descriptor::IsValid() const
{
    return Index != GfxInvalidBindlessIndex;
}

FResult FD3D12DescriptorHeap::Initialize(ID3D12Device* InDevice, const FD3D12DescriptorHeapDesc& InHeapDesc)
{
    if (InHeapDesc.Capacity == 0 || InHeapDesc.ReservedCount >= InHeapDesc.Capacity)
    {
        return RESULT_ERROR("Invalid descriptor heap capacity");
    }
    if (InHeapDesc.bIsShaderVisible && (InHeapDesc.HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || InHeapDesc.HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV))
    {
        return RESULT_ERROR("RTV and DSV descriptor heaps can't be shader visible");
    }

    const D3D12_DESCRIPTOR_HEAP_DESC NativeDesc = {
        .Type = InHeapDesc.HeapType,
        .NumDescriptors = InHeapDesc.Capacity,
        .Flags = InHeapDesc.bIsShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        .NodeMask = 0,
    };
    RESULT_TRY_PLATFORM_CALL(InDevice->CreateDescriptorHeap(&NativeDesc, IID_PPV_ARGS(&Heap)), "Failed to create descriptor heap")
    Heap->SetName(InHeapDesc.DebugName);

    HeapType = InHeapDesc.HeapType;
    Capacity = InHeapDesc.Capacity;
    ReservedCount = InHeapDesc.ReservedCount;
    bIsShaderVisible = InHeapDesc.bIsShaderVisible;
    DescriptorSize = InDevice->GetDescriptorHandleIncrementSize(HeapType);
    CpuStartHandle = Heap->GetCPUDescriptorHandleForHeapStart();
    GpuStartHandle = bIsShaderVisible ? Heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};

    std::scoped_lock Lock(Mutex);
    FreeIndices.clear();
    AllocationStates.assign(Capacity, 0);
    NextUnusedIndex = ReservedCount;
    AllocatedCount = 0;
    return RESULT_OK_EMPTY();
}

void FD3D12DescriptorHeap::Deinitialize()
{
    std::scoped_lock Lock(Mutex);
    if (AllocatedCount > 0)
    {
        RAVEN_LOG(LogD3D12, Warning, "Descriptor heap destroyed with {} descriptors still allocated", AllocatedCount);
    }
    FreeIndices.clear();
    AllocationStates.clear();
    NextUnusedIndex = 0;
    AllocatedCount = 0;
    Capacity = 0;
    ReservedCount = 0;
    Heap.Reset();
}

TResult<FD3D12Descriptor> FD3D12DescriptorHeap::Allocate()
{
    uint32 Index;
    {
        std::scoped_lock Lock(Mutex);
        if (!FreeIndices.empty())
        {
            Index = FreeIndices.back();
            FreeIndices.pop_back();
        }
        else if (NextUnusedIndex < Capacity)
        {
            Index = NextUnusedIndex++;
        }
        else
        {
            return RESULT_ERROR("Descriptor heap is full");
        }
        AllocationStates[Index] = 1;
        ++AllocatedCount;
    }
    return RESULT_OK(GetDescriptor(Index));
}

void FD3D12DescriptorHeap::Free(uint32 Index)
{
    if (Index == GfxInvalidBindlessIndex)
    {
        return;
    }

    std::scoped_lock Lock(Mutex);
    if (Index < ReservedCount || Index >= Capacity)
    {
        RAVEN_LOG(LogD3D12, Error, "Tried to free reserved or out-of-range index '{}' in descriptor heap", Index);
        return;
    }
    if (AllocationStates[Index] == 0)
    {
        RAVEN_LOG(LogD3D12, Error, "Double free of index '{}' in descriptor heap", Index);
        return;
    }
    AllocationStates[Index] = 0;
    --AllocatedCount;
    FreeIndices.push_back(Index);
}

void FD3D12DescriptorHeap::Free(const FD3D12Descriptor& Descriptor)
{
    Free(Descriptor.Index);
}

FD3D12Descriptor FD3D12DescriptorHeap::GetDescriptor(const uint32 Index) const
{
    FD3D12Descriptor Descriptor;
    Descriptor.Index = Index;
    Descriptor.CpuHandle.ptr = CpuStartHandle.ptr + static_cast<size64>(Index) * DescriptorSize;
    if (bIsShaderVisible)
    {
        Descriptor.GpuHandle.ptr = GpuStartHandle.ptr + static_cast<uint64>(Index) * DescriptorSize;
    }
    return Descriptor;
}

ID3D12DescriptorHeap* FD3D12DescriptorHeap::GetNative() const
{
    return Heap.Get();
}

uint32 FD3D12DescriptorHeap::GetCapacity() const
{
    return Capacity;
}

uint32 FD3D12DescriptorHeap::GetAllocatedCount() const
{
    std::scoped_lock Lock(Mutex);
    return AllocatedCount;
}
