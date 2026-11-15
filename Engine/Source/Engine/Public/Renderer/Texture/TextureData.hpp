// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Graphics/GfxTypes.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Memory/Containers/Vector.hpp"

struct FTextureMip
{
    uint32 Width = 0;
    uint32 Height = 0;
    TSpan<const byte8> Data;
};

struct FTextureData
{
    EGfxFormat Format = EGfxFormat::RGBA8_SRGB;
    TSpan<const FTextureMip> Mips;
};

struct FTextureBuilder
{
    uint32 Width = 0;
    uint32 Height = 0;
    EGfxFormat Format = EGfxFormat::RGBA8_SRGB;
    TVector<TVector<byte8>> Mips;

public:
    [[nodiscard]] FTextureData GetData()
    {
        MipViews.clear();
        for (uint32 Mip = 0; Mip < Mips.size(); ++Mip)
        {
            MipViews.push_back({.Width = GetMipSize(Width, Mip), .Height = GetMipSize(Height, Mip), .Data = Mips[Mip]});
        }
        return {.Format = Format, .Mips = MipViews};
    }

    [[nodiscard]] static constexpr uint32 GetMipSize(const uint32 Size, const uint32 Mip)
    {
        const uint32 Result = Size >> Mip;
        return Result > 0 ? Result : 1;
    }

private:
    TVector<FTextureMip> MipViews;
};
