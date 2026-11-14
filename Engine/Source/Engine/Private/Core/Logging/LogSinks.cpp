// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Core/Logging/LogSinks.hpp"

#include <algorithm>
#include <filesystem>

namespace
{
    constexpr size64 LineCapacity = FLog::MaxMessageLength + 256;

    FStringView GetColorCode(const ELogVerbosity Verbosity)
    {
        switch (Verbosity)
        {
        case ELogVerbosity::Fatal:
        case ELogVerbosity::Error: return "\x1b[91m";
        case ELogVerbosity::Warning: return "\x1b[93m";
        case ELogVerbosity::Info: return "\x1b[36m";
        case ELogVerbosity::Verbose: return "\x1b[37m";
        case ELogVerbosity::Trace: return "\x1b[90m";
        default:
            return {};
        }
    }

    constexpr FStringView ResetColorCode = "\x1b[0m";

    void Append(char* Buffer, size64& Length, const size64 Capacity, const FStringView Text)
    {
        const size64 Count = std::min(Text.size(), Capacity - Length);
        std::copy_n(Text.data(), Count, Buffer + Length);
        Length += Count;
    }
}

void FConsoleLogSink::Open()
{
    Output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (Output == nullptr || Output == INVALID_HANDLE_VALUE)
    {
        Output = nullptr;
        return;
    }

    DWORD Mode = 0;
    if (GetConsoleMode(Output, &Mode))
    {
        bUseColors = SetConsoleMode(Output, Mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
        SetConsoleOutputCP(CP_UTF8);
    }
}

void FConsoleLogSink::Write(const FLogRecord& Record)
{
    if (Output == nullptr)
    {
        return;
    }

    TChar Line[LineCapacity + 16];
    size64 Length = 0;

    if (bUseColors)
    {
        Append(Line, Length, sizeof(Line), GetColorCode(Record.Verbosity));
        const FStringView Prefix = FormatLogPrefix(Record, Line + Length, LineCapacity);
        Length += Prefix.size();
        Append(Line, Length, sizeof(Line), ResetColorCode);
        Append(Line, Length, sizeof(Line), Record.Message.substr(0, LineCapacity - Prefix.size()));
    }
    else
    {
        Length += FormatLogLine(Record, Line, LineCapacity).size();
    }
    Append(Line, Length, sizeof(Line), "\n");

    DWORD Written = 0;
    WriteFile(Output, Line, static_cast<DWORD>(Length), &Written, nullptr);
}

void FDebuggerLogSink::Write(const FLogRecord& Record)
{
    if (!IsDebuggerPresent())
    {
        return;
    }

    constexpr size64 PrefixCapacity = 512;
    TChar Line[PrefixCapacity + LineCapacity + 1];
    size64 Length = 0;

    if (Record.Verbosity <= ELogVerbosity::Warning)
    {
        const auto Result = std::format_to_n(Line, PrefixCapacity, "{}({}): ", Record.Location.file_name(), Record.Location.line());
        Length = std::min(static_cast<size64>(Result.size), PrefixCapacity);
    }
    const FStringView Text = FormatLogLine(Record, Line + Length, LineCapacity);
    Length += Text.size();
    Line[Length++] = '\n';

    TWideChar WideLine[std::size(Line) + 1];
    const int32 WideLength = MultiByteToWideChar(CP_UTF8, 0, Line, static_cast<int32>(Length), WideLine, static_cast<int32>(std::size(WideLine) - 1));
    WideLine[WideLength > 0 ? WideLength : 0] = L'\0';
    OutputDebugStringW(WideLine);
}
