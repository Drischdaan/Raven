// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cstring>
#include <type_traits>

#include "Graphics/GfxTypes.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Memory/Containers/Vector.hpp"

class IGfxCommandList;

class FUploadManager
{
public:
    void UploadBuffer(FGfxBufferHandle Destination, uint64 DestinationOffset, TSpan<const byte8> Data);

    template <typename T> requires std::is_trivially_copyable_v<T>
    void UploadBuffer(const FGfxBufferHandle Destination, const uint64 DestinationOffset, TSpan<const T> Data)
    {
        UploadBuffer(Destination, DestinationOffset, TSpan<const byte8>(reinterpret_cast<const byte8*>(Data.data()), Data.size_bytes()));
    }

    void Flush(IGfxCommandList& CommandList);

    [[nodiscard]] bool8 HasPendingUploads() const
    {
        return !PendingUploads.empty();
    }

    [[nodiscard]] uint64 GetPendingSize() const
    {
        return PendingData.size();
    }

private:
    struct FPendingUpload
    {
        FGfxBufferHandle Destination;
        uint64 DestinationOffset = 0;
        uint64 DataOffset = 0;
        uint64 Size = 0;
    };

    TVector<byte8> PendingData;
    TVector<FPendingUpload> PendingUploads;
};
