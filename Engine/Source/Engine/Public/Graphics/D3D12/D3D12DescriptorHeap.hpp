// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "D3D12CommandListPool.hpp"
#include "Core/CoreTypes.hpp"
#include "Core/Utility/Result.hpp"
#include "Graphics/GfxTypes.hpp"

struct RAVEN_API FD3D12Descriptor
{
    uint32 Index = GfxInvalidBindlessIndex;
    CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle = {};
    CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle = {};

public:
    [[nodiscard]] bool8 IsValid() const;
};

struct RAVEN_API FD3D12DescriptorHeapDesc
{
    D3D12_DESCRIPTOR_HEAP_TYPE HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    uint32 Capacity = 0;
    uint32 ReservedCount = 0;
    bool8 bIsShaderVisible = false;
    const TWideChar* DebugName = L"UnnamedDescriptorHeap";
};

class RAVEN_API FD3D12DescriptorHeap
{
public:
    FResult Initialize(ID3D12Device* InDevice, const FD3D12DescriptorHeapDesc& InHeapDesc);
    void Deinitialize();

    [[nodiscard]] TResult<FD3D12Descriptor> Allocate();
    void Free(uint32 Index);
    void Free(const FD3D12Descriptor& Descriptor);

    [[nodiscard]] FD3D12Descriptor GetDescriptor(uint32 Index) const;

    [[nodiscard]] ID3D12DescriptorHeap* GetNative() const;
    [[nodiscard]] uint32 GetCapacity() const;
    [[nodiscard]] uint32 GetAllocatedCount() const;

private:
    TComPtr<ID3D12DescriptorHeap> Heap;
    D3D12_DESCRIPTOR_HEAP_TYPE HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    CD3DX12_CPU_DESCRIPTOR_HANDLE CpuStartHandle = {};
    CD3DX12_GPU_DESCRIPTOR_HANDLE GpuStartHandle = {};
    uint32 DescriptorSize = 0;
    uint32 Capacity = 0;
    uint32 ReservedCount = 0;
    bool8 bIsShaderVisible = false;

    mutable std::mutex Mutex;
    TVector<uint32> FreeIndices;
    TVector<uint8> AllocationStates;
    uint32 NextUnusedIndex = 0;
    uint32 AllocatedCount = 0;
};
