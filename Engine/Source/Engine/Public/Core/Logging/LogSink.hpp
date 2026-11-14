// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once
#include <source_location>

#include "LogCategory.hpp"
#include "Core/CoreTypes.hpp"
#include "Core/Memory/Containers/String.hpp"

struct FLogTime
{
    uint16 Year = 0;
    uint16 Month = 0;
    uint16 Day = 0;
    uint16 Hour = 0;
    uint16 Minute = 0;
    uint16 Second = 0;
    uint16 Millisecond = 0;
};

struct FLogRecord
{
    FLogTime Time;
    uint32 ThreadId = 0;
    FStringView ThreadName;
    const FLogCategory* Category = nullptr;
    ELogVerbosity Verbosity = ELogVerbosity::Info;
    std::source_location Location;
    FStringView Message;
};

class RAVEN_API ILogSink
{
public:
    virtual ~ILogSink() = default;

    virtual void Write(const FLogRecord& Record) = 0;

    virtual void Flush()
    {
    }
};

[[nodiscard]] RAVEN_API FStringView FormatLogPrefix(const FLogRecord& Record, TChar* Buffer, size64 Capacity);
[[nodiscard]] RAVEN_API FStringView FormatLogLine(const FLogRecord& Record, TChar* Buffer, size64 Capacity);
