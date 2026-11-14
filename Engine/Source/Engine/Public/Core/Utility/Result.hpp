// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <assert.h>
#include <expected>
#include <intsafe.h>
#include <source_location>
#include <type_traits>
#include <utility>

#include "Core/CoreTypes.hpp"
#include "Core/Memory/Containers/String.hpp"
#include "Core/Utility/Macros.hpp"

struct RAVEN_API FRavenError
{
    uint64 PlatformCode = 0;
    std::source_location Location;
    FString Message;
};

[[nodiscard]] std::unexpected<FRavenError> MakeRavenError(FStringView Message = {}, std::source_location Location = std::source_location::current());
[[nodiscard]] std::unexpected<FRavenError> MakeRavenPlatformError(FStringView Message = {}, std::source_location Location = std::source_location::current());
[[nodiscard]] std::unexpected<FRavenError> MakeRavenPlatformError(uint64 PlatformCode, FStringView Message = {}, std::source_location Location = std::source_location::current());
[[nodiscard]] std::unexpected<FRavenError> MakeRavenPlatformError(long PlatformCode, FStringView Message = {}, std::source_location Location = std::source_location::current());

template <typename T>
class [[nodiscard]] TResult : public std::expected<T, FRavenError>
{
public:
    using Base = std::expected<T, FRavenError>;
    using Base::Base;

    template <typename TOther> requires std::is_void_v<T> && !std::is_void_v<TOther>
    TResult(const TResult<TOther>& Other)
        : Base(Other.HasValue() ? Base() : Base(std::unexpect, Other.error()))
    {
    }

    template <typename TOther> requires std::is_void_v<T> && !std::is_void_v<TOther>
    TResult(TResult<TOther>&& Other)
        : Base(Other.HasValue() ? Base() : Base(std::unexpect, std::move(Other).error()))
    {
    }

    template <typename TOther> requires std::is_convertible_v<TOther, T> && !std::is_same_v<TOther, T>
    explicit(!std::is_convertible_v<TOther, T>) TResult(const TResult<TOther>& Other)
        : Base(Other.HasValue() ? Base(Other.GetValue()) : Base(std::unexpect, Other.error()))
    {
    }

    template <typename TOther> requires std::is_convertible_v<TOther, T> && !std::is_same_v<TOther, T>
    explicit(!std::is_convertible_v<TOther, T>) TResult(TResult<TOther>&& Other)
        : Base(Other.HasValue() ? Base(std::move(Other.GetValue())) : Base(std::unexpect, std::move(Other.error())))
    {
    }

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

#define RESULT_OK(...) __VA_ARGS__
#define RESULT_OK_EMPTY() FResult()
#define RESULT_ERROR(...) MakeRavenError(__VA_ARGS__)
#define RESULT_PLATFORM_ERROR(...) MakeRavenPlatformError(__VA_ARGS__)

#define RESULT_TRY_PLATFORM_CALL(Call, ...) \
    long RAVEN_UNIQUE_VARIABLE(NativeResult) = Call; \
    if (RAVEN_UNIQUE_VARIABLE(NativeResult) < 0) \
    { \
        return RESULT_PLATFORM_ERROR(RAVEN_UNIQUE_VARIABLE(NativeResult) __VA_OPT__(, __VA_ARGS__)); \
    }

#define RESULT_TRY_PLATFORM_CALL_CLEANUP(Call, CleanUp, ...) \
    long RAVEN_UNIQUE_VARIABLE(NativeResult) = Call; \
    if (RAVEN_UNIQUE_VARIABLE(NativeResult) < 0) \
    { \
        CleanUp \
        return RESULT_PLATFORM_ERROR(RAVEN_UNIQUE_VARIABLE(NativeResult) __VA_OPT__(, __VA_ARGS__)); \
    }

// Both macros return the error as std::unexpected, so they work in a function returning any TResult type.
#define RESULT_CHECK(Method) \
    auto RAVEN_UNIQUE_VARIABLE(Result) = Method; \
    if (RAVEN_UNIQUE_VARIABLE(Result).HasError()) \
    { \
        return std::unexpected(std::move(RAVEN_UNIQUE_VARIABLE(Result)).error()); \
    }

#define RESULT_CHECK_ASSIGN(Method, VariableName) \
    auto RAVEN_UNIQUE_VARIABLE(Result) = Method; \
    if (RAVEN_UNIQUE_VARIABLE(Result).HasError()) \
    { \
        return std::unexpected(std::move(RAVEN_UNIQUE_VARIABLE(Result)).error()); \
    } \
    auto VariableName = std::move(RAVEN_UNIQUE_VARIABLE(Result)).GetValue();
