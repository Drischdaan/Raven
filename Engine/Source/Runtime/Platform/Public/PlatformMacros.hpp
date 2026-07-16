// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreMacros.hpp"
#include "CorePlatform.hpp"

#define PLATFORM_COMPILED_HEADER(Suffix) STRINGIFY(RAVEN_PLATFORM_NAME_RAW/CONCAT(RAVEN_PLATFORM_NAME_RAW, Suffix))
#define PLATFORM_COMPILED_HEADER_PATH(Path, Suffix) STRINGIFY(RAVEN_PLATFORM_NAME_RAW/Path/CONCAT(RAVEN_PLATFORM_NAME_RAW, Suffix))
