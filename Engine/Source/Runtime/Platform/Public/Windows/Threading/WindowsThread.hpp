// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <Windows.h>

#include "CoreTypes.hpp"

#include "Threading/ThreadTypes.hpp"

class PLATFORM_API FWindowsThread
{
public:
    FWindowsThread() = default;
    FWindowsThread(const FThreadDesc& InThreadDesc);
    FWindowsThread(const FWindowsThread&) = delete;
    FWindowsThread(FWindowsThread&& Other) noexcept;
    ~FWindowsThread();

    FWindowsThread& operator=(const FWindowsThread&) = delete;
    FWindowsThread& operator=(FWindowsThread&& Other) noexcept;

public:
    [[nodiscard]] bool8 IsValid() const;
    [[nodiscard]] bool8 IsJoinable() const;

    [[nodiscard]] uint64 GetThreadId() const;

    void Join();
    void Detach();

private:
    FThreadDesc ThreadDesc = {};

    HANDLE ThreadHandle = INVALID_HANDLE_VALUE;
    uint64 ThreadId = 0;
};

using FPlatformThread = FWindowsThread;
