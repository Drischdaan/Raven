// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once
#include <memory>
#include <type_traits>

#include "Core/CoreTypes.hpp"

/**
 * @note The virtual memory arena never calls object destructors. Use it for trivially destructible types, or destroy objects manually before resetting/popping past them. 
 */
class RAVEN_API FVirtualMemoryArena
{
public:
    static constexpr size64 DefaultCommitBlockSize = 64 * 1024;

public:
    FVirtualMemoryArena() = default;
    explicit FVirtualMemoryArena(size64 InReserveSize, size64 InCommitBlockSize = DefaultCommitBlockSize);
    FVirtualMemoryArena(const FVirtualMemoryArena&) = delete;
    FVirtualMemoryArena(FVirtualMemoryArena&& Other) noexcept;
    ~FVirtualMemoryArena();

    FVirtualMemoryArena& operator=(const FVirtualMemoryArena&) = delete;
    FVirtualMemoryArena& operator=(FVirtualMemoryArena&& Other) noexcept;

public:
    bool8 Initialize(size64 InReserveSize, size64 InCommitBlockSize = DefaultCommitBlockSize);
    void Deinitialize();

    [[nodiscard]] void* Allocate(size64 Size, size64 Alignment = 16);

    [[nodiscard]] size64 GetMarker() const;
    void PopToMarker(size64 Marker);
    void Reset(bool bShouldDecommit = false);

    [[nodiscard]] bool8 IsInitialized() const;
    [[nodiscard]] void* GetBasePointer() const;
    [[nodiscard]] size64 GetUsedSize() const;
    [[nodiscard]] size64 GetCommittedSize() const;
    [[nodiscard]] size64 GetReservedSize() const;
    [[nodiscard]] bool8 Owns(const void* Pointer) const;

public:
    template <typename T, typename... TArguments> requires std::is_constructible_v<T, TArguments...>
    [[nodiscard]] T* NewObject(TArguments&&... Arguments)
    {
        T* Pointer = static_cast<T*>(Allocate(sizeof(T), alignof(T)));
        if (Pointer == nullptr)
        {
            return nullptr;
        }
        return std::construct_at<T>(Pointer, std::forward<TArguments>(Arguments)...);
    }

private:
    uint8* BasePointer = nullptr;
    size64 ReservedSize = 0;
    size64 CommittedSize = 0;
    size64 CommittedBlockSize = 0;
    size64 Offset = 0;
};

struct RAVEN_API FScopedArenaMarker
{
    FVirtualMemoryArena& Arena;
    size64 Marker = 0;

public:
    FScopedArenaMarker(FVirtualMemoryArena& InArena);
    FScopedArenaMarker(FVirtualMemoryArena& InArena, size64 InMarker);
    ~FScopedArenaMarker();
};
