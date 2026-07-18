// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreCompiler.hpp"
#include "CorePlatform.hpp"
#include "CoreTypes.hpp"

#include "Utility/Atomic.hpp"

#if RAVEN_PLATFORM_WINDOWS
#   include <Windows.h>
#   if RAVEN_COMPILER_MSVC
#       pragma comment(lib, "Synchronization.lib") // WaitOnAddress / WakeByAddress*
#   endif
#endif

namespace AtomicWait
{
#if RAVEN_PLATFORM_WINDOWS
    inline void Wait(TAtomic<uint32>& Value, uint32 ExpectedValue)
    {
        WaitOnAddress(&Value, &ExpectedValue, sizeof(uint32), INFINITE);
    }

    inline void WakeOne(TAtomic<uint32>& Value)
    {
        WakeByAddressSingle(&Value);
    }

    inline void WakeAll(TAtomic<uint32>& Value)
    {
        WakeByAddressAll(&Value);
    }
#endif
}
