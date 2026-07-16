// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"

#include <Windows.h>

class PLATFORM_API FWindowsMutex
{
public:
    FWindowsMutex();
    FWindowsMutex(const FWindowsMutex&) = delete;
    FWindowsMutex(FWindowsMutex&& Other) noexcept;
    ~FWindowsMutex();

    FWindowsMutex& operator=(const FWindowsMutex&) = delete;
    FWindowsMutex& operator=(FWindowsMutex&& Other) noexcept;

public:
    void Lock();
    [[nodiscard]] bool8 TryLock();
    void Unlock();

private:
    SRWLOCK Mutex = SRWLOCK_INIT;

    friend class FWindowsConditionVariable;
};

using FPlatformMutex = FWindowsMutex;
