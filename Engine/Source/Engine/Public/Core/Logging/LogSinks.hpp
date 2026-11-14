// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <Windows.h>

#include "Core/Logging/Log.hpp"

class FConsoleLogSink : public ILogSink
{
public:
    void Open();
    void Write(const FLogRecord& Record) override;

private:
    HANDLE Output = nullptr;
    bool8 bUseColors = false;
};

class FDebuggerLogSink : public ILogSink
{
public:
    void Write(const FLogRecord& Record) override;
};
