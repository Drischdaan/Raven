// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "PlatformMacros.hpp"
#include "ThreadingConcepts.hpp"

#include "Mutex.hpp"

#include PLATFORM_COMPILED_HEADER_PATH(Threading, ConditionVariable.hpp)

using FConditionVariable = FPlatformConditionVariable;

static_assert(CConditionVariable<FConditionVariable, FMutex>, "Condition variable implementation is incomplete");

template <typename TPredicate>
void WaitUntil(FConditionVariable& ConditionVariable, FMutex& Mutex, TPredicate Predicate)
{
    while (!Predicate())
    {
        ConditionVariable.Wait(Mutex);
    }
}

template <typename TPredicate>
[[nodiscard]] bool8 WaitFor(FConditionVariable& ConditionVariable, FMutex& Mutex, const std::chrono::milliseconds Timeout, TPredicate Predicate)
{
    const std::chrono::steady_clock::time_point Deadline = std::chrono::steady_clock::now() + Timeout;
    while (!Predicate())
    {
        const std::chrono::milliseconds TimeRemaining = std::chrono::duration_cast<std::chrono::milliseconds>(Deadline - std::chrono::steady_clock::now());
        if (TimeRemaining <= std::chrono::milliseconds::zero() || !ConditionVariable.Wait(Mutex, TimeRemaining))
        {
            return Predicate();
        }
    }
    return true;
}
