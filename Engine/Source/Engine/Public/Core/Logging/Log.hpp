// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "LogSink.hpp"
#include "Core/CoreTypes.hpp"
#include "Core/Utility/Result.hpp"

struct FLogSettings
{
};

class RAVEN_API FLog
{
public:
    static constexpr size64 MaxMessageLength = 2048;
    static constexpr uint32 MaxSinks = 8;

public:
    static void Initialize(const FLogSettings& Settings = {});
    static void Shutdown();

    static bool8 AddSink(ILogSink& Sink);
    static void RemoveSink(ILogSink& Sink);
    static void Flush();

    [[noreturn]] static void HandleFatal();

    template <typename TCategory>
    [[nodiscard]] static consteval bool8 IsCompiledIn(const ELogVerbosity Verbosity)
    {
        return Verbosity == ELogVerbosity::Fatal || (Verbosity <= TCategory::CompiledVerbosity && Verbosity <= GLogMaxCompiledVerbosity);
    }

    template <typename... TArguments>
    static void Write(const FLogCategory& Category, const ELogVerbosity Verbosity, const std::source_location& Location, TFormatString<TArguments...> Format, TArguments&&... Arguments)
    {
        TChar Buffer[MaxMessageLength];
        const auto Result = std::format_to_n(Buffer, static_cast<std::ptrdiff_t>(MaxMessageLength), Format, std::forward<TArguments>(Arguments)...);
        size64 Length = static_cast<size64>(Result.size);
        if (Length > MaxMessageLength)
        {
            Length = MaxMessageLength;
            Buffer[Length - 3] = '.';
            Buffer[Length - 2] = '.';
            Buffer[Length - 1] = '.';
        }
        Dispatch(Category, Verbosity, Location, FStringView(Buffer, Length));
    }

    static void WriteError(const FLogCategory& Category, ELogVerbosity Verbosity, const FRavenError& Error);

    static void Dispatch(const FLogCategory& Category, ELogVerbosity Verbosity, const std::source_location& Location, FStringView Message);
};

#define RAVEN_LOG(Category, Verbosity, Format, ...) \
    do \
    { \
        if constexpr (FLog::IsCompiledIn<std::remove_cvref_t<decltype(Category)>>(ELogVerbosity::Verbosity)) \
        { \
            if ((Category).IsEnabled(ELogVerbosity::Verbosity)) \
            { \
                FLog::Write((Category), ELogVerbosity::Verbosity, std::source_location::current(), Format __VA_OPT__(, ) __VA_ARGS__); \
            } \
        } \
        if constexpr (ELogVerbosity::Verbosity == ELogVerbosity::Fatal) \
        { \
            FLog::HandleFatal(); \
        } \
    } \
    while (false)

#define RAVEN_LOG_ERROR(Category, ErrorObject) \
    do \
    { \
        if ((Category).IsEnabled(ELogVerbosity::Error)) \
        { \
            FLog::WriteError((Category), ELogVerbosity::Error, (ErrorObject)); \
        } \
    } \
    while (false)

#define RAVEN_FATAL(Category, Format, ...) \
    RAVEN_LOG(Category, Fatal, Format __VA_OPT__(, ) __VA_ARGS__)
