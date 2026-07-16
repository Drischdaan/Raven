// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Windows/Threading/WindowsMutex.hpp"

FWindowsMutex::FWindowsMutex()
{
    InitializeSRWLock(&Mutex);
}

FWindowsMutex::FWindowsMutex(FWindowsMutex&& Other) noexcept
{
    Mutex = Other.Mutex;
    Other.Mutex = SRWLOCK_INIT;
}

FWindowsMutex::~FWindowsMutex()
{
    Mutex = SRWLOCK_INIT;
}

FWindowsMutex& FWindowsMutex::operator=(FWindowsMutex&& Other) noexcept
{
    if (this != &Other)
    {
        Mutex = Other.Mutex;
        Other.Mutex = SRWLOCK_INIT;
    }
    return *this;
}

void FWindowsMutex::Lock()
{
    AcquireSRWLockExclusive(&Mutex);
}

bool8 FWindowsMutex::TryLock()
{
    return TryAcquireSRWLockExclusive(&Mutex) != 0;
}

void FWindowsMutex::Unlock()
{
    ReleaseSRWLockExclusive(&Mutex);
}
