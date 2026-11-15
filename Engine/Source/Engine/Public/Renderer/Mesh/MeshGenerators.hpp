// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "MeshData.hpp"

namespace MeshGenerators
{
    [[nodiscard]] RAVEN_API FMeshBuilder Cube(float32 HalfExtent = 0.5f);
    [[nodiscard]] RAVEN_API FMeshBuilder Sphere(float32 Radius = 0.5f, uint32 Segments = 32, uint32 Rings = 16);
    [[nodiscard]] RAVEN_API FMeshBuilder Plane(float32 HalfExtent = 0.5f, uint32 Subdivisions = 1);
}
