// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Core/Math/Vector2.hpp"
#include "Core/Math/Vector3.hpp"
#include "Core/Math/Vector4.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Memory/Containers/Vector.hpp"

struct FMeshData
{
    TSpan<const FVector3> Positions;
    TSpan<const FVector3> Normals;
    TSpan<const FVector4> Tangents;
    TSpan<const FVector2> TexCoords;
    TSpan<const uint32> Indices;
};

struct FMeshBuilder
{
    TVector<FVector3> Positions;
    TVector<FVector3> Normals;
    TVector<FVector4> Tangents;
    TVector<FVector2> TexCoords;
    TVector<uint32> Indices;

public:
    [[nodiscard]] FMeshData GetData() const
    {
        return {.Positions = Positions, .Normals = Normals, .Tangents = Tangents, .TexCoords = TexCoords, .Indices = Indices};
    }
};
