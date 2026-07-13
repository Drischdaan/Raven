// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

// []==================================================[]
// This file contains math types and functions that wrap
// glm. This might be a temporary solution, as I plan to
// implement a custom math library for this engine.
// For now the glm wrapper is in place.
//
// IMPORTANT: DON'T USE GLM DIRECTLY. Only use the
// provided types of this file. If something is missing,
// please add it here.
// []==================================================[]

#include <numbers>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "CoreTypes.hpp"

template <typename TScalar>
concept CScalar = std::is_floating_point_v<TScalar> && !std::is_same_v<TScalar, bool8>;

using FVector2 = glm::vec2;
using FVector3 = glm::vec3;
using FVector4 = glm::vec4;

using FQuaternion = glm::quat;

using FMatrix4 = glm::mat4;

static_assert(sizeof(FVector2) == 8 && std::is_trivially_copyable_v<FVector2>);
static_assert(sizeof(FVector3) == 12 && std::is_trivially_copyable_v<FVector3>);
static_assert(sizeof(FVector4) == 16 && std::is_trivially_copyable_v<FVector4>);

static_assert(sizeof(FQuaternion) == 16 && std::is_trivially_copyable_v<FQuaternion>);
static_assert(sizeof(FMatrix4) == 64 && std::is_trivially_copyable_v<FMatrix4>);

namespace Math
{
    template <CScalar TScalar>
    inline constexpr TScalar KindaSmallNumber = TScalar(1e-4);

    template <CScalar TScalar>
    [[nodiscard]] constexpr TScalar IsNearlyEqual(const TScalar A, const TScalar B, const TScalar Tolerance = KindaSmallNumber<TScalar>)
    {
        return Abs(A - B) <= Tolerance;
    }

    template <CScalar TScalar>
    [[nodiscard]] constexpr TScalar IsNearlyZero(const TScalar Value, const TScalar Tolerance = KindaSmallNumber<TScalar>)
    {
        return Abs(Value) <= Tolerance;
    }
}
