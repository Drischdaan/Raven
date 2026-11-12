// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <assert.h>
#include <expected>
#include <source_location>

#include "Core/CoreTypes.hpp"
#include "Core/Memory/Containers/String.hpp"

struct RAVEN_API FRavenError
{
    uint32 PlatformCode = 0;
    std::source_location Location;
    FString Message;
};

[[nodiscard]] std::unexpected<FRavenError> MakeRavenError(FStringView Message = {}, std::source_location Location = std::source_location::current());
[[nodiscard]] std::unexpected<FRavenError> MakeRavenPlatformError(FStringView Message = {}, std::source_location Location = std::source_location::current());

template <typename T>
class [[nodiscard]] TResult : public std::expected<T, FRavenError>
{
public:
    using Base = std::expected<T, FRavenError>;
    using Base::Base;

public:
    [[nodiscard]] bool8 HasValue() const
    {
        return this->has_value();
    }

    [[nodiscard]] bool8 HasError() const
    {
        return !HasValue();
    }

    [[nodiscard]] decltype(auto) GetValue() &
    {
        assert(HasValue() && "Unable to retrieve the value of a error result");
        return **this;
    }

    [[nodiscard]] decltype(auto) GetValue() &&
    {
        assert(HasValue() && "Unable to retrieve the value of a error result");
        return std::move(**this);
    }

    [[nodiscard]] const FRavenError& GetError() const &
    {
        assert(HasError() && "Unable to retrieve the value of a error result");
        return this->error();
    }

    void value() = delete;
};

using FResult = TResult<void>;

#define OK(...) FResult(__VA_ARGS__)
#define ERROR(...) MakeRavenError(__VA_ARGS__)
#define PLATFORM_ERROR(...) MakeRavenPlatformError(__VA_ARGS__)
