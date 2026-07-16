// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Windows/Threading/WindowsConditionVariable.hpp"

FWindowsConditionVariable::FWindowsConditionVariable()
{
    InitializeConditionVariable(&ConditionVariable);
}

FWindowsConditionVariable::FWindowsConditionVariable(FWindowsConditionVariable&& Other) noexcept
{
    ConditionVariable = Other.ConditionVariable;
    Other.ConditionVariable = CONDITION_VARIABLE_INIT;
}

FWindowsConditionVariable::~FWindowsConditionVariable()
{
    ConditionVariable = CONDITION_VARIABLE_INIT;
}

FWindowsConditionVariable& FWindowsConditionVariable::operator=(FWindowsConditionVariable&& Other) noexcept
{
    if (this != &Other)
    {
        ConditionVariable = Other.ConditionVariable;
        Other.ConditionVariable = CONDITION_VARIABLE_INIT;
    }
    return *this;
}

void FWindowsConditionVariable::Wait(FMutex& Mutex)
{
    SleepConditionVariableSRW(&ConditionVariable, &Mutex.Mutex, INFINITE, 0);
}

bool8 FWindowsConditionVariable::Wait(FMutex& Mutex, const std::chrono::milliseconds Timeout)
{
    return SleepConditionVariableSRW(&ConditionVariable, &Mutex.Mutex, static_cast<DWORD>(Timeout.count()), 0) != FALSE;
}

void FWindowsConditionVariable::NotifyOne()
{
    WakeConditionVariable(&ConditionVariable);
}

void FWindowsConditionVariable::NotifyAll()
{
    WakeAllConditionVariable(&ConditionVariable);
}
