// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Core/Logging/Log.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <mutex>

#include <Windows.h>

#include "Core/Logging/LogSinks.hpp"
#include "Core/Utility/Result.hpp"

namespace
{
    std::mutex CategoryRegistryMutex;
    std::mutex SinkMutex;
    ILogSink* Sinks[FLog::MaxSinks] = {};
    uint32 SinkCount = 0;

    FConsoleLogSink ConsoleSink;
    FDebuggerLogSink DebuggerSink;

    thread_local char ThreadName[32] = {};
    thread_local bool8 bIsDispatching = false;

    FStringView GetThreadName()
    {
        if (ThreadName[0] == '\0')
        {
            const auto Result = std::format_to_n(ThreadName, sizeof(ThreadName) - 1, "{}", GetCurrentThreadId());
            *Result.out = '\0';
        }
        return ThreadName;
    }

    void WriteFallback(const FLogRecord& Record)
    {
        char Line[FLog::MaxMessageLength + 258];
        const FStringView Text = FormatLogLine(Record, Line, FLog::MaxMessageLength + 256);
        Line[Text.size()] = '\n';
        Line[Text.size() + 1] = '\0';
        OutputDebugStringA(Line);
        std::fputs(Line, stderr);
    }
}

FLogCategory* FLogCategory::ListHead = nullptr;

FLogCategory::FLogCategory(const char* InName, const ELogVerbosity InDefaultVerbosity)
    : Name(InName)
      , DisplayName(InName)
      , RuntimeVerbosity(InDefaultVerbosity)
{
    if (std::strncmp(Name, "Log", 3) == 0 && Name[3] != '\0')
    {
        DisplayName = Name + 3;
    }

    std::scoped_lock Lock(CategoryRegistryMutex);
    Next = ListHead;
    ListHead = this;
}

FLogCategory::~FLogCategory()
{
    std::scoped_lock Lock(CategoryRegistryMutex);
    for (FLogCategory** Link = &ListHead; *Link != nullptr; Link = &(*Link)->Next)
    {
        if (*Link == this)
        {
            *Link = Next;
            break;
        }
    }
}

FLogCategory* FLogCategory::Find(const FStringView CategoryName)
{
    std::scoped_lock Lock(CategoryRegistryMutex);
    for (FLogCategory* Category = ListHead; Category != nullptr; Category = Category->Next)
    {
        if (LogDetail::EqualsIgnoreCase(CategoryName, Category->Name) || LogDetail::EqualsIgnoreCase(CategoryName, Category->DisplayName))
        {
            return Category;
        }
    }
    return nullptr;
}

void FLogCategory::SetAllVerbosity(const ELogVerbosity Verbosity)
{
    std::scoped_lock Lock(CategoryRegistryMutex);
    for (FLogCategory* Category = ListHead; Category != nullptr; Category = Category->Next)
    {
        Category->SetVerbosity(Verbosity);
    }
}

FStringView FormatLogPrefix(const FLogRecord& Record, char* Buffer, const size64 Capacity)
{
    const auto Result = std::format_to_n(Buffer, static_cast<std::ptrdiff_t>(Capacity),
                                         "[{:02}:{:02}:{:02}.{:03}] [{}] [{}] [{}] ",
                                         static_cast<uint32>(Record.Time.Hour), static_cast<uint32>(Record.Time.Minute),
                                         static_cast<uint32>(Record.Time.Second), static_cast<uint32>(Record.Time.Millisecond),
                                         Record.ThreadName,
                                         Record.Category != nullptr ? Record.Category->GetDisplayName() : FStringView("?"),
                                         LogVerbosityToString(Record.Verbosity));
    return {Buffer, std::min(static_cast<size64>(Result.size), Capacity)};
}

FStringView FormatLogLine(const FLogRecord& Record, char* Buffer, const size64 Capacity)
{
    const FStringView Prefix = FormatLogPrefix(Record, Buffer, Capacity);
    const size64 MessageLength = std::min(Record.Message.size(), Capacity - Prefix.size());
    std::copy_n(Record.Message.data(), MessageLength, Buffer + Prefix.size());
    return {Buffer, Prefix.size() + MessageLength};
}

void FLog::Initialize(const FLogSettings& Settings)
{
    ConsoleSink.Open();
    AddSink(ConsoleSink);

    AddSink(DebuggerSink);
}

void FLog::Shutdown()
{
    std::scoped_lock Lock(SinkMutex);
    for (uint32 Index = 0; Index < SinkCount; ++Index)
    {
        Sinks[Index]->Flush();
        Sinks[Index] = nullptr;
    }
    SinkCount = 0;
}

bool8 FLog::AddSink(ILogSink& Sink)
{
    std::scoped_lock Lock(SinkMutex);
    if (SinkCount == MaxSinks)
    {
        return false;
    }
    Sinks[SinkCount++] = &Sink;
    return true;
}

void FLog::RemoveSink(ILogSink& Sink)
{
    std::scoped_lock Lock(SinkMutex);
    for (uint32 Index = 0; Index < SinkCount; ++Index)
    {
        if (Sinks[Index] == &Sink)
        {
            Sink.Flush();
            Sinks[Index] = Sinks[--SinkCount];
            Sinks[SinkCount] = nullptr;
            return;
        }
    }
}

void FLog::Flush()
{
    std::scoped_lock Lock(SinkMutex);
    for (uint32 Index = 0; Index < SinkCount; ++Index)
    {
        Sinks[Index]->Flush();
    }
}

void FLog::HandleFatal()
{
    Flush();
    if (IsDebuggerPresent())
    {
        __debugbreak();
    }
    std::abort();
}

void FLog::WriteError(const FLogCategory& Category, const ELogVerbosity Verbosity, const FRavenError& Error)
{
    TChar Message[MaxMessageLength];
    size64 Length;
    if (Error.PlatformCode != 0)
    {
        TChar SystemMessage[512] = {};
        const DWORD SystemLength = FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
            nullptr, static_cast<DWORD>(Error.PlatformCode), 0, SystemMessage, sizeof(SystemMessage), nullptr);
        FStringView SystemText(SystemMessage, SystemLength);
        while (!SystemText.empty() && (SystemText.back() == ' ' || SystemText.back() == '.'))
        {
            SystemText.remove_suffix(1);
        }

        const auto Result = std::format_to_n(Message, MaxMessageLength, "{} (0x{:08X}: {})",
                                             Error.Message, static_cast<uint32>(Error.PlatformCode),
                                             SystemText.empty() ? FStringView("unknown platform error") : SystemText);
        Length = std::min(static_cast<size64>(Result.size), MaxMessageLength);
    }
    else
    {
        Length = std::min(Error.Message.size(), MaxMessageLength);
        std::copy_n(Error.Message.data(), Length, Message);
    }

    Dispatch(Category, Verbosity, Error.Location, FStringView(Message, Length));
}

void FLog::Dispatch(const FLogCategory& Category, const ELogVerbosity Verbosity, const std::source_location& Location, const FStringView Message)
{
    SYSTEMTIME LocalTime = {};
    GetLocalTime(&LocalTime);

    const FLogRecord Record = {
        .Time = {
            .Year = LocalTime.wYear,
            .Month = static_cast<uint8>(LocalTime.wMonth),
            .Day = static_cast<uint8>(LocalTime.wDay),
            .Hour = static_cast<uint8>(LocalTime.wHour),
            .Minute = static_cast<uint8>(LocalTime.wMinute),
            .Second = static_cast<uint8>(LocalTime.wSecond),
            .Millisecond = LocalTime.wMilliseconds,
        },
        .ThreadId = GetCurrentThreadId(),
        .ThreadName = GetThreadName(),
        .Category = &Category,
        .Verbosity = Verbosity,
        .Location = Location,
        .Message = Message,
    };

    if (bIsDispatching)
    {
        WriteFallback(Record);
        return;
    }

    bIsDispatching = true;
    {
        std::scoped_lock Lock(SinkMutex);
        if (SinkCount == 0)
        {
            WriteFallback(Record);
        }
        for (uint32 Index = 0; Index < SinkCount; ++Index)
        {
            Sinks[Index]->Write(Record);
        }
        if (Verbosity <= ELogVerbosity::Error)
        {
            for (uint32 Index = 0; Index < SinkCount; ++Index)
            {
                Sinks[Index]->Flush();
            }
        }
    }
    bIsDispatching = false;
}
