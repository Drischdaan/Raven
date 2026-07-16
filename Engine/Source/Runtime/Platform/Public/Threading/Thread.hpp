// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"
#include "PlatformMacros.hpp"
#include "ThreadingConcepts.hpp"

#include PLATFORM_COMPILED_HEADER_PATH(Threading, Thread.hpp)

using FThread = FPlatformThread;

static_assert(CThread<FThread>, "Mutex implementation is incomplete");
