// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <mutex>
#include <optional>
#include <type_traits>
#include <atomic>

#include "Core/CoreTypes.hpp"
#include "Core/Memory/Memory.hpp"
#include "Core/Memory/Containers/Array.hpp"
#include "Core/Memory/Containers/Vector.hpp"
#include "Core/Utility/Result.hpp"

template <typename THandle, typename TResource, uint32 TMaxCapacity = 1u << 20>
class TGfxResourcePool
{
public:
    static constexpr uint32 SlotsPerPage = 1024;
    static constexpr uint32 MaxCapacity = TMaxCapacity;
    static constexpr uint32 PageCount = (TMaxCapacity + SlotsPerPage - 1) / SlotsPerPage;

    static_assert(TMaxCapacity > 0 && TMaxCapacity < UINT32_MAX, "UINT32_MAX is the invalid handle index");

private:
    struct FSlot
    {
        std::atomic<uint32> Generation = 0;
        std::optional<TResource> Resource;
    };

    struct FPage
    {
        TArray<FSlot, SlotsPerPage> Slots;
    };

public:
    TGfxResourcePool() = default;
    TGfxResourcePool(const TGfxResourcePool&) = delete;

    ~TGfxResourcePool()
    {
        Clear();
    }

    TGfxResourcePool& operator=(const TGfxResourcePool&) = delete;

public:
    template <typename... TArguments> requires std::is_constructible_v<TResource, TArguments...>
    [[nodiscard]] TResult<THandle> Allocate(TArguments&&... Arguments)
    {
        std::scoped_lock Lock(Lock);

        uint32 Index;
        if (!FreeIndices.empty())
        {
            Index = FreeIndices.back();
            FreeIndices.pop_back();
        }
        else
        {
            if (NextUnusedIndex >= TMaxCapacity)
            {
                return RESULT_ERROR("Resource pool is full");
            }
            Index = NextUnusedIndex;

            const uint32 PageIndex = Index / SlotsPerPage;
            if (Pages[PageIndex].load(std::memory_order_relaxed) == nullptr)
            {
                FPage* NewPage = FMemory::NewObject<FPage>();
                if (NewPage == nullptr)
                {
                    return RESULT_ERROR("Out of memory while growing resource pool");
                }
                Pages[PageIndex].store(NewPage, std::memory_order_release);
            }
            ++NextUnusedIndex;
        }

        FSlot& Slot = GetSlotUnchecked(Index);
        Slot.Resource.emplace(std::forward<TArguments>(Arguments)...);

        const uint32 Generation = Slot.Generation.load(std::memory_order_relaxed) + 1;
        Slot.Generation.store(Generation, std::memory_order_release);
        ++AliveCount;

        THandle Handle;
        Handle.Index = Index;
        Handle.Generation = Generation;
        return RESULT_OK(Handle);
    }

    [[nodiscard]] TResult<TResource> Remove(const THandle Handle)
    {
        std::scoped_lock Lock(Lock);

        FSlot* Slot = FindSlot(Handle);
        if (Slot == nullptr)
        {
            return RESULT_ERROR("Invalid or stale resource handle");
        }
        Slot->Generation.store(Handle.Generation + 1, std::memory_order_release);

        TResource Resource = std::move(*Slot->Resource);
        Slot->Resource.reset();
        FreeIndices.push_back(Handle.Index);
        --AliveCount;
        return RESULT_OK(std::move(Resource));
    }

    [[nodiscard]] TResource* Get(const THandle Handle)
    {
        FSlot* Slot = FindSlot(Handle);
        return Slot != nullptr ? Slot->Resource : nullptr;
    }

    [[nodiscard]] const TResource* Get(const THandle Handle) const
    {
        return const_cast<TGfxResourcePool*>(this)->Get(Handle);
    }

    [[nodiscard]] bool8 Contains(const THandle Handle) const
    {
        return Get(Handle) != nullptr;
    }

    [[nodiscard]] uint32 GetAliveCount() const
    {
        std::scoped_lock Lock(Lock);
        return AliveCount;
    }

    template <typename TFunction>
    void ForEachAlive(TFunction&& Function)
    {
        std::scoped_lock Lock(Lock);

        TFunction TempFunction = std::forward<TFunction>(Function);
        for (uint32 Index = 0; Index < NextUnusedIndex; ++Index)
        {
            FSlot& Slot = GetSlotUnchecked(Index);
            const uint32 Generation = Slot.Generation.load(std::memory_order_relaxed);
            if ((Generation & 1u) == 0)
            {
                continue;
            }
            THandle Handle;
            Handle.Index = Index;
            Handle.Generation = Generation;
            TempFunction(Handle, *Slot.Resource);
        }
    }

    void Clear()
    {
        std::scoped_lock Lock(Lock);
        for (std::atomic<FPage*>& PageSlot : Pages)
        {
            FPage* Page = PageSlot.exchange(nullptr, std::memory_order_acq_rel);
            FMemory::DeleteObject(Page);
        }
        FreeIndices.clear();
        NextUnusedIndex = 0;
        AliveCount = 0;
    }

private:
    [[nodiscard]] FSlot* FindSlot(const THandle Handle) const
    {
        if (Handle.Index >= TMaxCapacity || (Handle.Generation & 1u) == 0)
        {
            return nullptr;
        }
        FPage* Page = Pages[Handle.Index / SlotsPerPage].load(std::memory_order_acquire);
        if (Page == nullptr)
        {
            return nullptr;
        }
        FSlot& Slot = Page->Slots[Handle.Index % SlotsPerPage];
        if (Slot.Generation.load(std::memory_order_acquire) != Handle.Generation)
        {
            return nullptr;
        }
        return &Slot;
    }

    [[nodiscard]] FSlot& GetSlotUnchecked(const uint32 Index)
    {
        return Pages[Index / SlotsPerPage].load(std::memory_order_relaxed)->Slots[Index % SlotsPerPage];
    }

private:
    TArray<std::atomic<FPage*>, PageCount> Pages = {};

    mutable std::mutex Mutex;
    TVector<uint32> FreeIndices;
    uint32 NextUnusedIndex = 0;
    uint32 AliveCount = 0;
};
