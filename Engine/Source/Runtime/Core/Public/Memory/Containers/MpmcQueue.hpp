// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"

#include "Utility/Atomic.hpp"

template <typename T, size64 TCapacity>
class TMpmcQueue
{
private:
    struct FCell
    {
        TAtomic<size64> Sequence;
        T Item;
    };

public:
    TMpmcQueue()
    {
        for (size64 Index = 0; Index < TCapacity; ++Index)
        {
            Cells[Index].Sequence.store(Index, std::memory_order_relaxed);
        }
    }

    TMpmcQueue(const TMpmcQueue&) = delete;
    TMpmcQueue(TMpmcQueue&&) = delete;
    ~TMpmcQueue() = default;

    TMpmcQueue& operator=(const TMpmcQueue&) = delete;
    TMpmcQueue& operator=(TMpmcQueue&&) = delete;

public:
    [[nodiscard]] bool8 TryEnqueue(const T& Item)
    {
        FCell* Cell;
        size64 Position = EnqueuePosition.load(std::memory_order_relaxed);
        for (;;)
        {
            Cell = &Cells[Position & (TCapacity - 1)];
            const int64 Delta = static_cast<int64>(Cell->Sequence.load(std::memory_order_acquire)) - static_cast<int64>(Position);
            if (Delta == 0)
            {
                if (EnqueuePosition.compare_exchange_weak(Position, Position + 1, std::memory_order_relaxed))
                {
                    break;
                }
            }
            else if (Delta < 0)
            {
                return false;
            }
            else
            {
                Position = EnqueuePosition.load(std::memory_order_relaxed);
            }
        }
        Cell->Item = Item;
        Cell->Sequence.store(Position + 1, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool8 TryDequeue(T& OutItem)
    {
        FCell* Cell;
        size64 Position = DequeuePosition.load(std::memory_order_relaxed);
        for (;;)
        {
            Cell = &Cells[Position & (TCapacity - 1)];
            const int64 Delta = static_cast<int64>(Cell->Sequence.load(std::memory_order_acquire)) - static_cast<int64>(Position + 1);
            if (Delta == 0)
            {
                if (DequeuePosition.compare_exchange_weak(Position, Position + 1, std::memory_order_relaxed))
                {
                    break;
                }
            }
            else if (Delta < 0)
            {
                return false;
            }
            else
            {
                Position = DequeuePosition.load(std::memory_order_relaxed);
            }
        }
        OutItem = Cell->Item;
        Cell->Sequence.store(Position + TCapacity, std::memory_order_release);
        return true;
    }

private:
    alignas(64) FCell Cells[TCapacity];
    alignas(64) TAtomic<size64> EnqueuePosition = 0;
    alignas(64) TAtomic<size64> DequeuePosition = 0;
};
