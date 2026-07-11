// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

// IMPORTANT: This should only be used in places where the HAL (Platform Abstraction Layer) is not available,
// such as in the engine's core code. It is not intended for use in user-facing code or
// in code that interacts with the platform directly.

#if RAVEN_ENABLE_ASSERTS
#   include <cassert>

#   define RAVEN_CORE_ASSERT(Expression, ...) assert(Expression __VA_OPT__( && __VA_ARGS__))
#   define RAVEN_CORE_ASSERT_PERSISTENT(Expression, ...) assert(Expression __VA_OPT__( && __VA_ARGS__))
#else
#   define RAVEN_CORE_ASSERT(Expression, ...)
#   define RAVEN_CORE_ASSERT_PERSISTENT(Expression, ...) Expression
#endif
