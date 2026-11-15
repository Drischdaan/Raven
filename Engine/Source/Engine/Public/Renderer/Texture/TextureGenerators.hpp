// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "TextureData.hpp"
#include "Core/Math/Color.hpp"

namespace TextureGenerators
{
    [[nodiscard]] RAVEN_API FTextureBuilder Solid(FColor Color, EGfxFormat Format = EGfxFormat::RGBA8_SRGB);
    [[nodiscard]] RAVEN_API FTextureBuilder Checker(uint32 Size, uint32 Cells, FColor A, FColor B);

    [[nodiscard]] RAVEN_API FTextureBuilder BumpNormalMap(uint32 Size, uint32 Bumps, float32 Strength = 1.0f);

    RAVEN_API void GenerateMips(FTextureBuilder& Texture, bool8 bNormalMap = false);
}
