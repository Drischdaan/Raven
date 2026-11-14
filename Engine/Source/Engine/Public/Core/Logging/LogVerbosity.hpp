// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Core/CoreTypes.hpp"
#include "Core/Memory/Containers/String.hpp"

enum class ELogVerbosity : uint8
{
    Fatal,
    Error,
    Warning,
    Info,
    Verbose,
    Trace,
};

#ifdef RAVEN_MODE_SHIPPING
inline constexpr ELogVerbosity GLogMaxCompiledVerbosity = ELogVerbosity::Info;
#else
inline constexpr ELogVerbosity GLogMaxCompiledVerbosity = ELogVerbosity::Trace;
#endif

[[nodiscard]] constexpr FStringView LogVerbosityToString(const ELogVerbosity Verbosity)
{
    switch (Verbosity)
    {
    case ELogVerbosity::Fatal: return "Fatal";
    case ELogVerbosity::Error: return "Error";
    case ELogVerbosity::Warning: return "Warning";
    case ELogVerbosity::Info: return "Info";
    case ELogVerbosity::Verbose: return "Verbose";
    case ELogVerbosity::Trace: return "Trace";
    default:
        return "Unknown";
    }
}

namespace LogDetail
{
    [[nodiscard]] constexpr TChar ToLower(const TChar Character)
    {
        return (Character >= 'A' && Character <= 'Z') ? static_cast<char>(Character + ('a' - 'A')) : Character;
    }

    [[nodiscard]] constexpr bool8 EqualsIgnoreCase(const FStringView A, const FStringView B)
    {
        if (A.size() != B.size())
        {
            return false;
        }
        for (size64 Index = 0; Index < A.size(); ++Index)
        {
            if (ToLower(A[Index]) != ToLower(B[Index]))
            {
                return false;
            }
        }
        return true;
    }
}

[[nodiscard]] constexpr bool8 TryParseLogVerbosity(const FStringView Text, ELogVerbosity& OutVerbosity)
{
    constexpr ELogVerbosity AllVerbosities[] = {
        ELogVerbosity::Fatal, ELogVerbosity::Error, ELogVerbosity::Warning, ELogVerbosity::Info, ELogVerbosity::Verbose, ELogVerbosity::Trace,
    };
    for (const ELogVerbosity Verbosity : AllVerbosities)
    {
        if (LogDetail::EqualsIgnoreCase(Text, LogVerbosityToString(Verbosity)))
        {
            OutVerbosity = Verbosity;
            return true;
        }
    }
    return false;
}
