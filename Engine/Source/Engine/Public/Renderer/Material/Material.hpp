// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Renderer/RendererTypes.hpp"
#include "Core/Math/Color.hpp"
#include "Core/Utility/EnumFlags.hpp"

enum class EBlendMode : uint8
{
    Opaque,
    Masked,
    Translucent,
};

enum class EMaterialFlags : uint32
{
    None = 0,
    TwoSided = 1 << 0,
    Unlit = 1 << 1,
};

RAVEN_ENUM_FLAGS(EMaterialFlags)

struct FMaterialDesc
{
    FLinearColor BaseColor = {.R = 1.0f, .G = 1.0f, .B = 1.0f, .A = 1.0f};
    float32 Metallic = 0.0f;
    float32 Roughness = 0.5f;
    FLinearColor Emissive = {.R = 0.0f, .G = 0.0f, .B = 0.0f, .A = 1.0f};
    float32 NormalScale = 1.0f;
    float32 AlphaCutoff = 0.5f;

    FTextureHandle BaseColorTexture;
    FTextureHandle NormalTexture;
    FTextureHandle MetallicRoughnessTexture;
    FTextureHandle EmissiveTexture;

    EBlendMode BlendMode = EBlendMode::Opaque;
    EMaterialFlags Flags = EMaterialFlags::None;
};
