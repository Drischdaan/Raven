// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreMacros.hpp"
#include "CoreTypes.hpp"

#include "Memory/Containers/String.hpp"

#if RAVEN_PROFILER_ENABLED
#   include <tracy/Tracy.hpp>
#endif

namespace Profiler
{
    struct FZone
    {
        const TChar* Name;
        uint32 Color;
    };

    namespace Colors
    {
        inline constexpr uint32 White = 0xFFFFFFFF;
        inline constexpr uint32 Black = 0x00000000;
        inline constexpr uint32 Red = 0xFF0000FF;
        inline constexpr uint32 Green = 0xFF00FF00;
        inline constexpr uint32 Blue = 0xFFFF0000;
        inline constexpr uint32 Yellow = 0xFFFFFF00;
        inline constexpr uint32 Magenta = 0xFFFF00FF;
        inline constexpr uint32 Cyan = 0xFF00FFFF;
        inline constexpr uint32 Gray = 0xFF808080;
        inline constexpr uint32 Orange = 0xFFFFA500;
        inline constexpr uint32 Purple = 0xFF800080;
        inline constexpr uint32 Brown = 0xFFA52A2A;
        inline constexpr uint32 Pink = 0xFFFFC0CB;

        constexpr uint32 CreateColor(const uint8 R, const uint8 G, const uint8 B)
        {
            return (static_cast<uint32>(R) << 24) | (static_cast<uint32>(G) << 16) | (static_cast<uint32>(B) << 8);
        }
    }
}

#if RAVEN_PROFILER_ENABLED

#   define RAVEN_DEFINE_PROFILER_ZONE(Name, Color) inline constexpr Profiler::FZone CONCAT(Name, Zone) = { (#Name), (Color) }

#   define RAVEN_PROFILE_SRCLOC(Definition) \
        static constexpr tracy::SourceLocationData CONCAT(RavenProfilerSrcLoc, __LINE__) = { (Definition.Name), __FUNCTION__, __FILE__, static_cast<uint32>(__LINE__), (Definition).Color }

#   define RAVEN_PROFILE_FUNCTION() \
        static constexpr tracy::SourceLocationData CONCAT(RavenProfilerSrcLoc, __LINE__) = { nullptr, __FUNCTION__, __FILE__, static_cast<uint32>(__LINE__) }; \
        tracy::ScopedZone CONCAT(RavenProfilerZone, __LINE__) = { &CONCAT(RavenProfilerSrcLoc, __LINE__), true }

#   define RAVEN_PROFILE_FUNCTION_COLOR(ColorValue) \
        static constexpr tracy::SourceLocationData CONCAT(RavenProfilerSrcLoc, __LINE__) = { nullptr, __FUNCTION__, __FILE__, static_cast<uint32>(__LINE__), (ColorValue) }; \
        tracy::ScopedZone CONCAT(RavenProfilerZone, __LINE__) = { &CONCAT(RavenProfilerSrcLoc, __LINE__), true }

#   define RAVEN_PROFILE_ZONE(Definition) \
        RAVEN_PROFILE_SRCLOC(Definition); \
        tracy::ScopedZone CONCAT(RavenProfilerZone, __LINE__) = { &CONCAT(RavenProfilerSrcLoc, __LINE__), true }

#   define RAVEN_PROFILE_ZONE_DYNAMIC(Definition, SuffixText, SuffixLength) \
        RAVEN_PROFILE_ZONE(Definition); \
        CONCAT(RavenProfilerZone, __LINE__).Name((SuffixText), (SuffixLength))

#   define RAVEN_PROFILE_ZONE_TEXT(Definition, Text, TextLength) \
        RAVEN_PROFILE_ZONE(Definition); \
        CONCAT(RavenProfilerZone, __LINE__).Text((Text), (TextLength))

#   define RAVEN_PROFILE_FRAME_MARK() FrameMark
#   define RAVEN_PROFILE_FRAME_MARK_NAMED(Name) FrameMarkNamed((Name))
#   define RAVEN_PROFILE_THREAD(Name) tracy::SetThreadName((Name))
#   define RAVEN_PROFILE_PLOT(Name, Value) TracyPlot((Name), (Value))

#   define RAVEN_PROFILE_FIBER_ENTER(FiberName) TracyFiberEnter((FiberName))
#   define RAVEN_PROFILE_FIBER_ENTER_HINT(FiberName, GroupHint) TracyFiberEnterHint((FiberName), (GroupHint))
#   define RAVEN_PROFILE_FIBER_LEAVE() TracyFiberLeave

#else
#   define RAVEN_DEFINE_PROFILER_ZONE(...)
#   define RAVEN_PROFILE_SRCLOC(Definition)
#   define RAVEN_PROFILE_FUNCTION(...)               ((void)0)
#   define RAVEN_PROFILE_ZONE(...)               ((void)0)
#   define RAVEN_PROFILE_ZONE_DYNAMIC(...)               ((void)0)
#   define RAVEN_PROFILE_ZONE_TEXT(...)               ((void)0)
#   define RAVEN_PROFILE_FRAME_MARK(...)               ((void)0)
#   define RAVEN_PROFILE_FRAME_MARK_NAMED(...)               ((void)0)
#   define RAVEN_PROFILE_THREAD(...)               ((void)0)
#   define RAVEN_PROFILE_PLOT(...)               ((void)0)
#endif
