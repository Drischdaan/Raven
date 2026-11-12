// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <atomic>

#include "Containers/String.hpp"
#include "Core/CoreTypes.hpp"

enum class EMemoryTag : uint8
{
    Unknown = 0,
    Core = 1,
    Strings = 2,
    Count,
};

struct FMemoryTagStats
{
    std::atomic<int64> AllocatedBytes = 0;
    std::atomic<int64> PeakAllocatedBytes = 0;
    std::atomic<int64> AllocationCount = 0;
    std::atomic<int64> DeallocationCount = 0;
};

namespace MemoryTag
{
    RAVEN_API void PushTag(EMemoryTag Tag);
    RAVEN_API void PopTag();

    [[nodiscard]] RAVEN_API EMemoryTag GetCurrentTag();
    [[nodiscard]] RAVEN_API const FMemoryTagStats& GetMemoryTagStats(EMemoryTag Tag);
    [[nodiscard]] const TChar* GetMemoryTagName(EMemoryTag Tag);
}

class RAVEN_API FMemoryTagScope
{
public:
    explicit FMemoryTagScope(EMemoryTag InTag);
    FMemoryTagScope(const FMemoryTagScope&) = delete;
    FMemoryTagScope(FMemoryTagScope&&) = delete;
    ~FMemoryTagScope();

    FMemoryTagScope& operator=(const FMemoryTagScope&) = delete;
    FMemoryTagScope& operator=(FMemoryTagScope&&) = delete;
};

#define RAVEN_MEMORY_PRIVATE_CONCAT_INNER(A, B) A##B
#define RAVEN_MEMORY_PRIVATE_CONCAT(A, B) RAVEN_MEMORY_PRIVATE_CONCAT_INNER(A, B)

#define RAVEN_MEMORY_SCOPE(Tag) const FMemoryTagScope RAVEN_MEMORY_PRIVATE_CONCAT(MemoryScope, __COUNTER__)(EMemoryTag::Tag)
