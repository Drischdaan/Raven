// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <format>

#include "Core/Math/Quat.hpp"
#include "Core/Math/Vector2.hpp"
#include "Core/Math/Vector3.hpp"
#include "Core/Math/Vector4.hpp"

namespace LogDetail
{
    template <typename TComponentFormatter, typename TContext, typename... TComponents>
    auto FormatComponents(const TComponentFormatter& Formatter, TContext& Context, const TComponents&... Components)
    {
        auto Out = Context.out();
        *Out++ = '(';
        bool8 bIsFirst = true;
        ((Out = [&]
        {
            if (!bIsFirst)
            {
                *Out++ = ',';
                *Out++ = ' ';
            }
            bIsFirst = false;
            Context.advance_to(Out);
            return Formatter.format(Components, Context);
        }()), ...);
        *Out++ = ')';
        return Out;
    }
}

template <typename T>
struct std::formatter<TVector2<T>> : std::formatter<T>
{
    template <typename TContext>
    auto format(const TVector2<T>& Vector, TContext& Context) const
    {
        return LogDetail::FormatComponents(static_cast<const std::formatter<T>&>(*this), Context, Vector.X, Vector.Y);
    }
};

template <typename T>
struct std::formatter<TVector3<T>> : std::formatter<T>
{
    template <typename TContext>
    auto format(const TVector3<T>& Vector, TContext& Context) const
    {
        return LogDetail::FormatComponents(static_cast<const std::formatter<T>&>(*this), Context, Vector.X, Vector.Y, Vector.Z);
    }
};

template <typename T>
struct std::formatter<TVector4<T>> : std::formatter<T>
{
    template <typename TContext>
    auto format(const TVector4<T>& Vector, TContext& Context) const
    {
        return LogDetail::FormatComponents(static_cast<const std::formatter<T>&>(*this), Context, Vector.X, Vector.Y, Vector.Z, Vector.W);
    }
};

template <>
struct std::formatter<FQuat> : std::formatter<float32>
{
    template <typename TContext>
    auto format(const FQuat& Quat, TContext& Context) const
    {
        return LogDetail::FormatComponents(static_cast<const std::formatter<float32>&>(*this), Context, Quat.X, Quat.Y, Quat.Z, Quat.W);
    }
};
