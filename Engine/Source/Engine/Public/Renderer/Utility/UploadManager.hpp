// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cstring>
#include <type_traits>

#include "Graphics/GfxBarriers.hpp"
#include "Graphics/GfxTypes.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Memory/Containers/Vector.hpp"
#include "Core/Utility/Result.hpp"

class IGfxDevice;
class IGfxCommandList;

class FUploadManager
{
public:
    void Initialize(IGfxDevice& InDevice);

    void UploadBuffer(FGfxBufferHandle Destination, uint64 DestinationOffset, TSpan<const byte8> Data);

    template <typename T> requires std::is_trivially_copyable_v<T>
    void UploadBuffer(const FGfxBufferHandle Destination, const uint64 DestinationOffset, TSpan<const T> Data)
    {
        UploadBuffer(Destination, DestinationOffset, TSpan<const byte8>(reinterpret_cast<const byte8*>(Data.data()), Data.size_bytes()));
    }

    FResult UploadTexture(FGfxTextureHandle Destination, uint32 Subresource, TSpan<const byte8> Data);

    void Flush(IGfxCommandList& CommandList);

    [[nodiscard]] bool8 HasPendingUploads() const
    {
        return !PendingBufferUploads.empty() || !PendingTextureUploads.empty();
    }

    [[nodiscard]] uint64 GetPendingSize() const
    {
        return PendingData.size();
    }

private:
    struct FPendingBufferUpload
    {
        FGfxBufferHandle Destination;
        uint64 DestinationOffset = 0;
        uint64 DataOffset = 0;
        uint64 Size = 0;
    };

    struct FPendingTextureUpload
    {
        FGfxTextureHandle Destination;
        uint32 Subresource = 0;
        uint64 DataOffset = 0;
    };

    uint64 Reserve(uint64 Size, uint64 Alignment);

private:
    IGfxDevice* Device = nullptr;
    TVector<byte8> PendingData;
    TVector<FPendingBufferUpload> PendingBufferUploads;
    TVector<FPendingTextureUpload> PendingTextureUploads;
    TVector<FGfxTextureBarrier> TextureBarriers;
};
