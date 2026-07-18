// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "JobSystem/SpinLock.hpp"

void FSpinLock::Lock()
{
    for (;;)
    {
        if (!Flag.exchange(true, std::memory_order_acquire))
        {
            return;
        }
        while (Flag.load(std::memory_order_relaxed))
        {
            _mm_pause();
        }
    }
}

bool8 FSpinLock::TryLock()
{
    return !Flag.exchange(true, std::memory_order_acquire);
}

void FSpinLock::Unlock()
{
    Flag.store(false, std::memory_order_release);
}
