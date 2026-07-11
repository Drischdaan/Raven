// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <memory>
#include <type_traits>

#include "CorePlatform.hpp"
#include "CoreTypes.hpp"

class CORE_API FLinearAllocator
{
public:
    explicit FLinearAllocator(uint64 InReservedSize);
    FLinearAllocator(const FLinearAllocator&) = delete;
    ~FLinearAllocator() noexcept;

    FLinearAllocator& operator=(const FLinearAllocator&) = delete;

public:
    [[nodiscard]] void* Allocate(uint64 Size, uint64 Alignment = RAVEN_PLATFORM_POINTER_ALIGNMENT) noexcept;

    void Reset() noexcept;

    [[nodiscard]] size64 GetOffset() const noexcept;
    [[nodiscard]] size64 GetHighWaterMarker() const noexcept;

public:
    template <typename T, typename... TArguments> requires std::is_constructible_v<T, TArguments...>
    [[nodiscard]] T* New(TArguments&&... Arguments) noexcept
    {
        return std::construct_at(static_cast<T*>(Allocate(sizeof(T), alignof(T))), std::forward<TArguments>(Arguments)...);
    }

protected:
    void Rollback(size64 Mark) noexcept;

protected:
    uint64 ReservedSize = 0;

    uint8* BasePointer = nullptr;
    uint64 CommitedSize = 0;
    size64 Offset = 0;
    size64 HighWaterMark = 0;
};
