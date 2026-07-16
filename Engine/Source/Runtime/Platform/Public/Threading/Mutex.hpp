// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "PlatformMacros.hpp"
#include "ThreadingConcepts.hpp"

#include PLATFORM_COMPILED_HEADER_PATH(Threading, Mutex.hpp)

using FMutex = FPlatformMutex;

static_assert(CMutex<FMutex>, "Mutex implementation is incomplete");
