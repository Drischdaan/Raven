// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cstring>
#include <memory>

#include "D3D12Include.hpp"
#include "Core/Utility/Result.hpp"

class RAVEN_API FD3D12PipelineStream
{
public:
    static constexpr size64 Capacity = 1024;

public:
    template <typename TSubobject>
    void Add(const TSubobject& Subobject)
    {
        static_assert(alignof(TSubobject) <= alignof(void*), "Subobjects are pointer aligned");
        const size64 Offset = (Size + alignof(void*) - 1) & ~(alignof(void*) - 1);
        if (Offset + sizeof(TSubobject) > Capacity)
        {
            bOverflowed = true;
            return;
        }
        std::memcpy(Data + Offset, std::addressof(Subobject), sizeof(TSubobject));
        Size = Offset + sizeof(TSubobject);
    }

    [[nodiscard]] TResult<D3D12_PIPELINE_STATE_STREAM_DESC> GetDesc()
    {
        if (bOverflowed)
        {
            return RESULT_ERROR("Pipeline state stream overflowed; raise FD3D12PipelineStream::Capacity");
        }
        return RESULT_OK(D3D12_PIPELINE_STATE_STREAM_DESC{ .SizeInBytes = Size, .pPipelineStateSubobjectStream = Data });
    }

private:
    alignas(void*) byte8 Data[Capacity] = {};
    size64 Size = 0;
    bool8 bOverflowed = false;
};
