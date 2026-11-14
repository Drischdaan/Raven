// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <mutex>

#include "Core/CoreTypes.hpp"
#include "Core/Memory/Containers/Vector.hpp"
#include "Core/Utility/Result.hpp"
#include "Graphics/GfxResources.hpp"

class FD3D12GfxDevice;

class RAVEN_API FD3D12TransientAllocator
{
public:
    static constexpr uint64 PageSize = 4ull * 1024 * 1024;
    static constexpr uint32 MaxFrameSlots = 4;

public:
    void Initialize(FD3D12GfxDevice& InDevice);
    void Deinitialize();

    void BeginFrame(uint32 FrameSlot);

    [[nodiscard]] TResult<FGfxTransientAllocation> Allocate(uint64 Size, uint64 Alignment);

private:
    struct FPage
    {
        FGfxBufferHandle BufferHandle;
        byte8* CpuData = nullptr;
        uint64 Size = 0;
    };

    struct FFrame
    {
        TVector<FPage> Pages;
        TVector<FPage> DedicatedPages;
        uint32 PageIndex = 0;
        uint64 Offset = 0;
    };

private:
    [[nodiscard]] TResult<FPage> CreatePage(uint64 Size) const;
    void DestroyPages(TVector<FPage>& Pages) const;

private:
    [[nodiscard]] static FGfxTransientAllocation MakeAllocation(const FPage& Page, uint64 Offset, uint64 Size);

private:
    FD3D12GfxDevice* Device = nullptr;

    std::mutex Mutex;
    TArray<FFrame, MaxFrameSlots> Frames;
    uint32 CurrentSlot = 0;
};
