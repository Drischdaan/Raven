// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <chrono>
#include <Windows.h>

#include "Threading/Mutex.hpp"

class PLATFORM_API FWindowsConditionVariable
{
public:
    FWindowsConditionVariable();
    FWindowsConditionVariable(const FWindowsConditionVariable&) = delete;
    FWindowsConditionVariable(FWindowsConditionVariable&& Other) noexcept;
    ~FWindowsConditionVariable();

    FWindowsConditionVariable& operator=(const FWindowsConditionVariable&) = delete;
    FWindowsConditionVariable& operator=(FWindowsConditionVariable&& Other) noexcept;

public:
    void Wait(FMutex& Mutex);
    [[nodiscard]] bool8 Wait(FMutex& Mutex, std::chrono::milliseconds Timeout);

    void NotifyOne();
    void NotifyAll();

private:
    CONDITION_VARIABLE ConditionVariable = CONDITION_VARIABLE_INIT;
};

using FPlatformConditionVariable = FWindowsConditionVariable;
