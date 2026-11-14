// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Core/Math/MathUtility.hpp"
#include "Core/Math/Matrix4x4.hpp"
#include "Core/Math/Vector3.hpp"

struct FRenderView
{
    FMatrix4x4 ViewMatrix = FMatrix4x4::Identity();
    FVector3 Position = {};
    float32 VerticalFov = Math::DegreesToRadians(60.0f);
    float32 NearPlane = 0.1f;
};
