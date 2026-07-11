// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Memory/Allocators/LinearAllocator.hpp"

#include "Memory/Memory.hpp"
#include "snitch/snitch.hpp"

namespace
{
    // A comfortably large reservation so allocations never approach the arena limit.
    constexpr uint64 ReservedSize = 1024 * 1024;

    // Exposes the protected Rollback member for testing.
    class FTestLinearAllocator : public FLinearAllocator
    {
    public:
        using FLinearAllocator::FLinearAllocator;
        using FLinearAllocator::Rollback;
    };

    // Tracks constructor invocations for the New template test.
    struct Widget
    {
        static inline int32 ConstructCount = 0;

        int32 Value = 0;

        explicit Widget(const int32 InValue) noexcept : Value(InValue) { ++ConstructCount; }
    };

    // Returns the byte distance between two pointers returned by the allocator.
    size64 Distance(const void* Low, const void* High) noexcept
    {
        return static_cast<size64>(static_cast<const uint8*>(High) - static_cast<const uint8*>(Low));
    }
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST_CASE("FLinearAllocator starts empty", "[allocator][linear]")
{
    FLinearAllocator Allocator(ReservedSize);

    CHECK(Allocator.GetOffset() == 0);
    CHECK(Allocator.GetHighWaterMarker() == 0);
}

// ---------------------------------------------------------------------------
// Allocate
// ---------------------------------------------------------------------------

TEST_CASE("FLinearAllocator::Allocate returns usable, aligned memory", "[allocator][linear]")
{
    FLinearAllocator Allocator(ReservedSize);

    constexpr uint64 Size = 128;
    void* Pointer = Allocator.Allocate(Size);
    REQUIRE(Pointer != nullptr);
    CHECK(Memory::IsAligned(Pointer, RAVEN_PLATFORM_POINTER_ALIGNMENT));

    // The whole block must be writable.
    Memory::Set(Pointer, 0xCD, Size);
    CHECK(static_cast<uint8*>(Pointer)[0] == 0xCD);
    CHECK(static_cast<uint8*>(Pointer)[Size - 1] == 0xCD);
}

TEST_CASE("FLinearAllocator::Allocate advances the offset by the request size", "[allocator][linear]")
{
    FLinearAllocator Allocator(ReservedSize);

    (void)Allocator.Allocate(64);
    CHECK(Allocator.GetOffset() == 64);

    (void)Allocator.Allocate(32);
    CHECK(Allocator.GetOffset() == 96);
}

TEST_CASE("FLinearAllocator hands out distinct, sequential blocks", "[allocator][linear]")
{
    FLinearAllocator Allocator(ReservedSize);

    void* First = Allocator.Allocate(16);
    void* Second = Allocator.Allocate(16);
    void* Third = Allocator.Allocate(16);

    REQUIRE(First != nullptr);
    REQUIRE(Second != nullptr);
    REQUIRE(Third != nullptr);

    // Blocks advance forward with no overlap (16-byte requests at 16-byte alignment).
    CHECK(Distance(First, Second) == 16);
    CHECK(Distance(Second, Third) == 16);
}

TEST_CASE("FLinearAllocator::Allocate pads for over-aligned requests", "[allocator][linear]")
{
    FLinearAllocator Allocator(ReservedSize);

    void* First = Allocator.Allocate(1);
    REQUIRE(First != nullptr);
    CHECK(Allocator.GetOffset() == 1);

    constexpr uint64 Alignment = 64;
    void* Second = Allocator.Allocate(8, Alignment);
    REQUIRE(Second != nullptr);
    CHECK(Memory::IsAligned(Second, Alignment));

    // The second block is placed at the next 64-byte boundary after offset 1.
    CHECK(Distance(First, Second) == Alignment);
    CHECK(Allocator.GetOffset() == Alignment + 8);
}

TEST_CASE("FLinearAllocator commits enough memory to span multiple pages", "[allocator][linear]")
{
    FLinearAllocator Allocator(ReservedSize);

    // A single allocation larger than one page forces additional commits.
    const uint64 Size = Memory::GetPageSize() * 3;
    auto* Pointer = static_cast<uint8*>(Allocator.Allocate(Size));
    REQUIRE(Pointer != nullptr);

    Memory::Set(Pointer, 0x11, Size);
    CHECK(Pointer[0] == 0x11);
    CHECK(Pointer[Size - 1] == 0x11);
    CHECK(Allocator.GetOffset() == Size);
}

// ---------------------------------------------------------------------------
// High water mark
// ---------------------------------------------------------------------------

TEST_CASE("FLinearAllocator tracks the high water mark", "[allocator][linear]")
{
    FLinearAllocator Allocator(ReservedSize);

    (void)Allocator.Allocate(256);
    CHECK(Allocator.GetHighWaterMarker() == 256);

    (void)Allocator.Allocate(128);
    CHECK(Allocator.GetHighWaterMarker() == 384);
}

// ---------------------------------------------------------------------------
// Reset
// ---------------------------------------------------------------------------

TEST_CASE("FLinearAllocator::Reset rewinds the offset but keeps the high water mark", "[allocator][linear]")
{
    FLinearAllocator Allocator(ReservedSize);

    void* First = Allocator.Allocate(512);
    (void)Allocator.Allocate(512);
    CHECK(Allocator.GetOffset() == 1024);

    Allocator.Reset();
    CHECK(Allocator.GetOffset() == 0);
    CHECK(Allocator.GetHighWaterMarker() == 1024);

    // After a reset the arena reuses the same memory from the start.
    void* Reused = Allocator.Allocate(512);
    CHECK(Reused == First);
    CHECK(Allocator.GetHighWaterMarker() == 1024);
}

// ---------------------------------------------------------------------------
// Rollback
// ---------------------------------------------------------------------------

TEST_CASE("FLinearAllocator::Rollback restores a previous offset", "[allocator][linear]")
{
    FTestLinearAllocator Allocator(ReservedSize);

    (void)Allocator.Allocate(128);
    const size64 Mark = Allocator.GetOffset();

    void* Temporary = Allocator.Allocate(256);
    REQUIRE(Temporary != nullptr);
    CHECK(Allocator.GetOffset() == Mark + 256);

    Allocator.Rollback(Mark);
    CHECK(Allocator.GetOffset() == Mark);

    // The rolled-back space is reused by the next allocation.
    void* Reused = Allocator.Allocate(64);
    CHECK(Reused == Temporary);

    // Rollback does not lower the high water mark.
    CHECK(Allocator.GetHighWaterMarker() == Mark + 256);
}

// ---------------------------------------------------------------------------
// New
// ---------------------------------------------------------------------------

TEST_CASE("FLinearAllocator::New constructs an object in the arena", "[allocator][linear]")
{
    Widget::ConstructCount = 0;

    FLinearAllocator Allocator(ReservedSize);

    Widget* Object = Allocator.New<Widget>(99);
    REQUIRE(Object != nullptr);
    CHECK(Object->Value == 99);
    CHECK(Widget::ConstructCount == 1);
    CHECK(Memory::IsAligned(Object, alignof(Widget)));
    CHECK(Allocator.GetOffset() >= sizeof(Widget));
}
