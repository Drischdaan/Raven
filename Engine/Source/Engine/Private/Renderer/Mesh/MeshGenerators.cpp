// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Mesh/MeshGenerators.hpp"

#include "Core/Math/MathUtility.hpp"

namespace
{
    void AddGridFace(FMeshBuilder& Mesh, const FVector3& Normal, const FVector3& Up, const float32 HalfExtent, const float32 Offset, const uint32 Subdivisions)
    {
        const FVector3 Right = FVector3::Cross(Normal, Up);
        const FVector3 Center = Normal * Offset;
        const uint32 FirstVertex = static_cast<uint32>(Mesh.Positions.size());
        const uint32 RowLength = Subdivisions + 1;

        for (uint32 Row = 0; Row <= Subdivisions; ++Row)
        {
            for (uint32 Column = 0; Column <= Subdivisions; ++Column)
            {
                const float32 U = static_cast<float32>(Column) / static_cast<float32>(Subdivisions);
                const float32 V = static_cast<float32>(Row) / static_cast<float32>(Subdivisions);
                const FVector3 Position = Center + Right * ((U * 2.0f - 1.0f) * HalfExtent) + Up * ((1.0f - V * 2.0f) * HalfExtent);

                Mesh.Positions.push_back(Position);
                Mesh.Normals.push_back(Normal);
                Mesh.Tangents.push_back(FVector4::FromVector3(Right, 1.0f));
                Mesh.TexCoords.push_back({.X = U, .Y = V});
            }
        }

        for (uint32 Row = 0; Row < Subdivisions; ++Row)
        {
            for (uint32 Column = 0; Column < Subdivisions; ++Column)
            {
                const uint32 TopLeft = FirstVertex + Row * RowLength + Column;
                const uint32 TopRight = TopLeft + 1;
                const uint32 BottomLeft = TopLeft + RowLength;
                const uint32 BottomRight = BottomLeft + 1;
                Mesh.Indices.insert(Mesh.Indices.end(), {TopLeft, TopRight, BottomRight, TopLeft, BottomRight, BottomLeft});
            }
        }
    }
}

namespace MeshGenerators
{
    FMeshBuilder Cube(const float32 HalfExtent)
    {
        FMeshBuilder Mesh;
        AddGridFace(Mesh, FVector3::UnitX(), FVector3::UnitY(), HalfExtent, HalfExtent, 1);
        AddGridFace(Mesh, -FVector3::UnitX(), FVector3::UnitY(), HalfExtent, HalfExtent, 1);
        AddGridFace(Mesh, FVector3::UnitY(), FVector3::UnitZ(), HalfExtent, HalfExtent, 1);
        AddGridFace(Mesh, -FVector3::UnitY(), -FVector3::UnitZ(), HalfExtent, HalfExtent, 1);
        AddGridFace(Mesh, FVector3::UnitZ(), FVector3::UnitY(), HalfExtent, HalfExtent, 1);
        AddGridFace(Mesh, -FVector3::UnitZ(), FVector3::UnitY(), HalfExtent, HalfExtent, 1);
        return Mesh;
    }

    FMeshBuilder Plane(const float32 HalfExtent, const uint32 Subdivisions)
    {
        FMeshBuilder Mesh;
        AddGridFace(Mesh, FVector3::UnitY(), FVector3::UnitZ(), HalfExtent, 0.0f, Subdivisions < 1 ? 1 : Subdivisions);
        return Mesh;
    }

    FMeshBuilder Sphere(const float32 Radius, const uint32 Segments, const uint32 Rings)
    {
        FMeshBuilder Mesh;
        const uint32 SegmentCount = Segments < 3 ? 3 : Segments;
        const uint32 RingCount = Rings < 2 ? 2 : Rings;

        for (uint32 Ring = 0; Ring <= RingCount; ++Ring)
        {
            const float32 V = static_cast<float32>(Ring) / static_cast<float32>(RingCount);
            const float32 Phi = V * Math::Pi;
            for (uint32 Segment = 0; Segment <= SegmentCount; ++Segment)
            {
                const float32 U = static_cast<float32>(Segment) / static_cast<float32>(SegmentCount);
                const float32 Theta = U * Math::TwoPi;
                const FVector3 Normal = {
                    .X = Math::Sin(Phi) * Math::Cos(Theta),
                    .Y = Math::Cos(Phi),
                    .Z = Math::Sin(Phi) * Math::Sin(Theta),
                };

                Mesh.Positions.push_back(Normal * Radius);
                Mesh.Normals.push_back(Normal);
                Mesh.Tangents.push_back({.X = -Math::Sin(Theta), .Y = 0.0f, .Z = Math::Cos(Theta), .W = 1.0f});
                Mesh.TexCoords.push_back({.X = U, .Y = V});
            }
        }

        const uint32 RowLength = SegmentCount + 1;
        for (uint32 Ring = 0; Ring < RingCount; ++Ring)
        {
            for (uint32 Segment = 0; Segment < SegmentCount; ++Segment)
            {
                const uint32 TopLeft = Ring * RowLength + Segment;
                const uint32 TopRight = TopLeft + 1;
                const uint32 BottomLeft = TopLeft + RowLength;
                const uint32 BottomRight = BottomLeft + 1;
                if (Ring != 0)
                {
                    Mesh.Indices.insert(Mesh.Indices.end(), {TopLeft, TopRight, BottomRight});
                }
                if (Ring != RingCount - 1)
                {
                    Mesh.Indices.insert(Mesh.Indices.end(), {TopLeft, BottomRight, BottomLeft});
                }
            }
        }
        return Mesh;
    }
}
