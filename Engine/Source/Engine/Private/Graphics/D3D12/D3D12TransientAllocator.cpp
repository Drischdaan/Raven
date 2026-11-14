// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12TransientAllocator.hpp"

#include "Graphics/D3D12/D3D12Conversions.hpp"
#include "Graphics/D3D12/D3D12GfxDevice.hpp"

void FD3D12TransientAllocator::Initialize(FD3D12GfxDevice& InDevice)
{
    Device = &InDevice;
    CurrentSlot = 0;
}

void FD3D12TransientAllocator::Deinitialize()
{
    std::scoped_lock Lock(Mutex);
    for (FFrame& Frame : Frames)
    {
        DestroyPages(Frame.Pages);
        DestroyPages(Frame.DedicatedPages);
        Frame.PageIndex = 0;
        Frame.Offset = 0;
    }
}

void FD3D12TransientAllocator::BeginFrame(const uint32 FrameSlot)
{
    std::scoped_lock Lock(Mutex);
    CurrentSlot = FrameSlot;

    FFrame& Frame = Frames[CurrentSlot];
    Frame.PageIndex = 0;
    Frame.Offset = 0;
    DestroyPages(Frame.DedicatedPages);
}

TResult<FGfxTransientAllocation> FD3D12TransientAllocator::Allocate(const uint64 Size, const uint64 Alignment)
{
    if (Size == 0)
    {
        return RESULT_ERROR("Transient allocation size must not be zero");
    }
    if (!IsPowerOfTwo(Alignment) || Alignment > D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
    {
        return RESULT_ERROR("Transient allocation alignment must be a power of two up to 64 KB");
    }

    std::scoped_lock Lock(Mutex);
    FFrame& Frame = Frames[CurrentSlot];
    if (Size > PageSize)
    {
        RESULT_CHECK_ASSIGN(CreatePage(Size), DedicatedPage)
        Frame.DedicatedPages.push_back(DedicatedPage);
        return RESULT_OK(MakeAllocation(DedicatedPage, 0, Size));
    }

    while (true)
    {
        if (Frame.PageIndex < Frame.Pages.size())
        {
            const FPage& Page = Frame.Pages[Frame.PageIndex];
            const uint64 Offset = AlignUp(Frame.Offset, Alignment);
            if (Offset + Size <= Page.Size)
            {
                Frame.Offset = Offset + Size;
                return RESULT_OK(MakeAllocation(Page, Offset, Size));
            }
            ++Frame.PageIndex;
            Frame.Offset = 0;
            continue;
        }

        RESULT_CHECK_ASSIGN(CreatePage(PageSize), NewPage)
        Frame.Pages.push_back(NewPage);
    }
}

TResult<FD3D12TransientAllocator::FPage> FD3D12TransientAllocator::CreatePage(const uint64 Size) const
{
    const FGfxBufferDesc BufferDesc = {
        .Size = AlignUp(Size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT),
        .Stride = 0,
        .Usage = EGfxBufferUsage::Constant | EGfxBufferUsage::Vertex | EGfxBufferUsage::Index | EGfxBufferUsage::CopySource,
        .MemoryType = EGfxMemoryType::Upload,
    };
    RESULT_CHECK_ASSIGN(Device->CreateBuffer(BufferDesc, "TransientUploadPage"), BufferHandle)

    const TSpan<byte8> CpuData = Device->Map(BufferHandle);
    return RESULT_OK(FPage{ .BufferHandle = BufferHandle, .CpuData = CpuData.data(), .Size = BufferDesc.Size });
}

void FD3D12TransientAllocator::DestroyPages(TVector<FPage>& Pages) const
{
    for (const FPage& Page : Pages)
    {
        Device->DestroyBuffer(Page.BufferHandle);
    }
    Pages.clear();
}

FGfxTransientAllocation FD3D12TransientAllocator::MakeAllocation(const FPage& Page, const uint64 Offset, const uint64 Size)
{
    FGfxTransientAllocation Allocation;
    Allocation.CpuData = TSpan<byte8>(Page.CpuData + Offset, Size);
    Allocation.GpuRange = FGfxBufferRange{.BufferHandle = Page.BufferHandle, .Offset = Offset, .Size = Size};
    return Allocation;
}
