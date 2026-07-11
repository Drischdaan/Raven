// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreAssertions.hpp"
#include "Span.hpp"
#include "Vector.hpp"

#include "Memory/Handle.hpp"

/**
 * @brief Storage pool for handles
 * @tparam T Underlying type of the handle pool
 * @tparam TTag Tag type for handles
 */
template <typename T, typename TTag = T>
class TSparseHandlePool
{
protected:
    struct FSparseEntry
    {
        uint32 DenseIndex = 0;
        uint8 Generation = 0;
    };

public:
    using FHandle = THandle<TTag>;

    template <typename... TArguments>
    [[nodiscard]] FHandle Create(TArguments&&... Arguments)
    {
        uint32 Slot;
        if (FreeHead != InvalidSlot)
        {
            Slot = FreeHead;
            FreeHead = Sparse[Slot].DenseIndex;
        }
        else
        {
            Slot = static_cast<uint32>(Sparse.size());
            RAVEN_CORE_ASSERT(Slot < FHandle::MaxIndex, "Sparse handle pool is full");
            Sparse.push_back(FSparseEntry{});
        }

        Sparse[Slot].DenseIndex = static_cast<uint32>(Items.size());
        Items.emplace_back(std::forward<TArguments>(Arguments)...);
        DenseToSlot.push_back(Slot);
        return FHandle{
            .Index = Slot,
            .Generation = Sparse[Slot].Generation,
        };
    }

    void Destroy(FHandle Handle)
    {
        RAVEN_CORE_ASSERT(IsAlive(Handle), "Handle is not alive");

        FSparseEntry& SparseEntry = Sparse[Handle.Index];
        const uint32 DenseIndex = SparseEntry.DenseIndex;
        const uint32 LastItemIndex = static_cast<uint32>(Items.size()) - 1;

        if (DenseIndex != LastItemIndex)
        {
            Items[DenseIndex] = std::move(Items[LastItemIndex]);
            DenseToSlot[DenseIndex] = DenseToSlot[LastItemIndex];
            Sparse[DenseToSlot[DenseIndex]].DenseIndex = DenseIndex;
        }

        Items.pop_back();
        DenseToSlot.pop_back();

        SparseEntry.Generation = (SparseEntry.Generation + 1) & 0xFF; // This wraps every 256 recycles, which might cause a stale handle to be read as alive 
        SparseEntry.DenseIndex = FreeHead;
        FreeHead = Handle.Index;
    }

    [[nodiscard]] bool8 IsAlive(FHandle Handle) const noexcept
    {
        return Handle.IsValid() && Handle.Index < Sparse.size() && Sparse[Handle.Index].Generation == Handle.Generation;
    }

    [[nodiscard]] T& Resolve(FHandle Handle)
    {
        RAVEN_CORE_ASSERT(IsAlive(Handle), "Handle is not alive");

        return Items[Sparse[Handle.Index].DenseIndex];
    }

    [[nodiscard]] T* TryResolve(FHandle Handle)
    {
        return IsAlive(Handle) ? &Items[Sparse[Handle.Index].DenseIndex] : nullptr;
    }

    [[nodiscard]] TSpan<T> GetItems() { return Items; }
    [[nodiscard]] TSpan<const T> GetItems() const { return Items; }
    [[nodiscard]] size64 GetItemCount() const { return Items.size(); }

protected:
    static constexpr uint32 InvalidSlot = 0xFFFFFFFFu;

protected:
    TVector<T> Items;
    TVector<uint32> DenseToSlot;
    TVector<FSparseEntry> Sparse;
    uint32 FreeHead = InvalidSlot;
};
