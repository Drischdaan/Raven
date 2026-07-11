// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <memory>

#include "CoreTypes.hpp"

#include "Memory/Memory.hpp"

template <typename T, size64 TSlotsPerPage = 256>
class TPoolAllocator
{
private:
    union FSlot
    {
        alignas(T) uint8 Storage[sizeof(T)];
        FSlot* NextSlot;
    };

    struct FPage
    {
        FSlot Slots[TSlotsPerPage];
        FPage* NextPage;
    };

public:
    TPoolAllocator() = default;
    TPoolAllocator(const TPoolAllocator&) = delete;

    ~TPoolAllocator() noexcept
    {
        for (FPage* Page = Pages; Page != nullptr;)
        {
            FPage* NextPage = Page->NextPage;
            Memory::Free(Page, alignof(FPage));
            Page = NextPage;
        }
    }

    TPoolAllocator& operator=(const TPoolAllocator&) = delete;

public:
    template <typename... TArguments> requires std::is_constructible_v<T, TArguments...>
    [[nodiscard]] T* New(TArguments&&... Arguments) noexcept
    {
        if (FreeSlots == nullptr) [[unlikely]]
        {
            AddPage();
        }
        FSlot* Slot = FreeSlots;
        FreeSlots = Slot->NextSlot;
        return std::construct_at(reinterpret_cast<T*>(Slot->Storage), std::forward<TArguments>(Arguments)...);
    }

    void Delete(T* Object) noexcept
    {
        if (Object == nullptr) [[unlikely]]
        {
            return;
        }
        std::destroy_at(Object);
        FSlot* Slot = std::launder(reinterpret_cast<FSlot*>(Object));
        Slot->NextSlot = FreeSlots;
        FreeSlots = Slot;
    }

protected:
    void AddPage()
    {
        FPage* NewPage = static_cast<FPage*>(Memory::Allocate(sizeof(FPage), alignof(FPage)));
        std::construct_at(NewPage);
        NewPage->NextPage = Pages;
        Pages = NewPage;
        for (size64 Index = 0; Index < TSlotsPerPage; ++Index)
        {
            NewPage->Slots[Index].NextSlot = FreeSlots;
            FreeSlots = &NewPage->Slots[Index];
        }
    }

protected:
    FPage* Pages = nullptr;
    FSlot* FreeSlots = nullptr;
};
