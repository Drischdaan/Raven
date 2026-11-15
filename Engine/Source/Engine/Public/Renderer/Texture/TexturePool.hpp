// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "TextureData.hpp"
#include "Graphics/GfxResourcePool.hpp"
#include "Renderer/RendererTypes.hpp"
#include "Core/Memory/Containers/Array.hpp"
#include "Core/Memory/Containers/String.hpp"
#include "Core/Utility/Result.hpp"

class IGfxDevice;
class FUploadManager;

enum class EDefaultTexture : uint8
{
    White,
    Black,
    FlatNormal,
    Count,
};

class FTexturePool
{
public:
    static constexpr uint32 MaxTextures = 1u << 16;

    struct FTexture
    {
        FGfxTextureHandle GfxTexture;
        uint32 BindlessIndex = UINT32_MAX;
        uint32 Width = 0;
        uint32 Height = 0;
        uint32 MipCount = 0;
        EGfxFormat Format = EGfxFormat::Unknown;
        FString Name;
    };

public:
    FTexturePool() = default;
    FTexturePool(const FTexturePool&) = delete;
    FTexturePool& operator=(const FTexturePool&) = delete;
    ~FTexturePool();

public:
    FResult Initialize(IGfxDevice& InDevice, FUploadManager& InUploads);
    void Deinitialize();

    TResult<FTextureHandle> CreateTexture(const FTextureData& Data, FStringView Name);
    void DestroyTexture(FTextureHandle Handle);

    [[nodiscard]] const FTexture* GetTexture(FTextureHandle Handle) const
    {
        return Textures.Get(Handle);
    }

    [[nodiscard]] uint32 GetBindlessIndex(FTextureHandle Handle, EDefaultTexture Fallback) const;

    [[nodiscard]] FTextureHandle GetDefault(const EDefaultTexture Texture) const
    {
        return Defaults[static_cast<size64>(Texture)];
    }

private:
    IGfxDevice* Device = nullptr;
    FUploadManager* Uploads = nullptr;
    TGfxResourcePool<FTextureHandle, FTexture, MaxTextures> Textures;
    TArray<FTextureHandle, static_cast<size64>(EDefaultTexture::Count)> Defaults = {};
};
