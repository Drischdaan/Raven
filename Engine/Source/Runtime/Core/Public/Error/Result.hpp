// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <expected>
#include <source_location>

#include "CoreAssertions.hpp"
#include "CoreErrorCodes.hpp"
#include "CoreMacros.hpp"
#include "CoreTypes.hpp"

constexpr uint16 RAVEN_DOMAIN_UNKNOWN = 0;
constexpr uint16 RAVEN_DOMAIN_CORE = 1;

struct CORE_API FError
{
    uint16 Domain = RAVEN_DOMAIN_UNKNOWN; // Domain identifier
    uint16 Code = RAVEN_ERROR_CORE_UNKNOWN; // Domain-specific error code
    uint32 Payload = 0; // Native Error Code (HRESULT, VKResult, etc.)

#ifdef RAVEN_MODE_DEBUG
    std::source_location SourceLocation = std::source_location::current();
#endif
};

template <typename T>
class TResult : public std::expected<T, FError>
{
    using FSuper = std::expected<T, FError>;

public:
    using FSuper::FSuper;

public:
    [[nodiscard]] bool8 HasValue() const noexcept
    {
        return this->has_value();
    }

    [[nodiscard]] bool8 HasError() const noexcept
    {
        return !this->has_value();
    }

    [[nodiscard]] decltype(auto) GetValue(this auto&& Self)
    {
        RAVEN_CORE_ASSERT(Self.has_value(), "Trying to get value on an error result");
        return *std::forward_like<decltype(Self)>(Self);
    }

    [[nodiscard]] T ValueOr(T Fallback) const
    {
        return this->has_value() ? **this : std::move(Fallback);
    }

    [[nodiscard]] const FError& GetError() const
    {
        RAVEN_CORE_ASSERT(!this->has_value(), "Trying to get error on a value result");
        return this->error();
    }
};

#ifdef RAVEN_MODE_DEBUG
[[nodiscard]] inline std::unexpected<FError> MakeError(const uint16 Domain, const uint16 Code, const uint32 Payload = 0, const std::source_location& SourceLocation = std::source_location::current())
#else
[[nodiscard]] inline std::unexpected<FError> MakeError(const uint16 Domain, const uint16 Code, const uint32 Payload = 0)
#endif
{
    return std::unexpected(FError{
        .Domain = Domain,
        .Code = Code,
        .Payload = Payload,
#ifdef RAVEN_MODE_DEBUG
        .SourceLocation = SourceLocation,
#endif
    });
}

// Template specialization for void return types
template <>
class [[nodiscard]] TResult<void> : public std::expected<void, FError>
{
    using FSuper = std::expected<void, FError>;

public:
    using FSuper::FSuper;

public:
    [[nodiscard]] bool8 HasValue() const noexcept
    {
        return this->has_value();
    }

    [[nodiscard]] bool8 HasError() const noexcept
    {
        return !this->has_value();
    }

    [[nodiscard]] const FError& GetError() const
    {
        RAVEN_CORE_ASSERT(!this->has_value(), "Trying to get error on a value result");
        return this->error();
    }
};

#define RAVEN_TRY(VariableName, Expression) \
    auto VariableName##_Result = (Expression); \
    if (!VariableName##_Result) [[unlikely]] \
    { \
        return std::unexpected(std::move(VariableName##_Result).error());  \
    } \
    auto& VariableName = VariableName##_Result.GetValue()

#define RAVEN_TRY_VOID(Expression) \
    if (auto CONCAT(RavenTry_, __LINE__) = (Expression); !CONCAT(RavenTry_, __LINE__)) [[unlikely]] \
    { \
        return std::unexpected(std::move(CONCAT(RavenTry_, __LINE__)).error()); \
    }
