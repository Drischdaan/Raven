// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Memory/Memory.hpp"

#include "snitch/snitch.hpp"

namespace
{
    // Tracks constructor / destructor invocations for New / Delete tests.
    struct LifetimeProbe
    {
        static inline int32 ConstructCount = 0;
        static inline int32 DestructCount = 0;

        int32 Value = 0;

        LifetimeProbe() noexcept { ++ConstructCount; }
        explicit LifetimeProbe(const int32 InValue) noexcept : Value(InValue) { ++ConstructCount; }
        ~LifetimeProbe() noexcept { ++DestructCount; }

        LifetimeProbe(const LifetimeProbe&) = delete;
        LifetimeProbe& operator=(const LifetimeProbe&) = delete;

        static void Reset() noexcept
        {
            ConstructCount = 0;
            DestructCount = 0;
        }
    };

    // Type with an over-aligned requirement to exercise alignof-based allocation.
    struct alignas(64) OverAligned
    {
        uint8 Data[64];
    };
}

// ---------------------------------------------------------------------------
// Allocate / Free
// ---------------------------------------------------------------------------

TEST_CASE("Memory::Allocate returns a usable, aligned block", "[memory][allocate]")
{
    constexpr size64 Size = 128;

    void* Pointer = Memory::Allocate(Size);
    REQUIRE(Pointer != nullptr);
    CHECK(Memory::IsAligned(Pointer, RAVEN_PLATFORM_POINTER_ALIGNMENT));

    // The returned block must be writable across the full requested size.
    Memory::Set(Pointer, 0xAB, Size);
    CHECK(static_cast<uint8*>(Pointer)[0] == 0xAB);
    CHECK(static_cast<uint8*>(Pointer)[Size - 1] == 0xAB);

    Memory::Free(Pointer);
}

TEST_CASE("Memory::Allocate honours a custom alignment", "[memory][allocate]")
{
    constexpr size64 Alignment = 256;

    void* Pointer = Memory::Allocate(64, Alignment);
    REQUIRE(Pointer != nullptr);
    CHECK(Memory::IsAligned(Pointer, Alignment));

    Memory::Free(Pointer, Alignment);
}

TEST_CASE("Memory::GetAllocationSize is at least the requested size", "[memory][allocate]")
{
    constexpr size64 Size = 100;

    void* Pointer = Memory::Allocate(Size);
    REQUIRE(Pointer != nullptr);
    CHECK(Memory::GetAllocationSize(Pointer) >= Size);

    Memory::Free(Pointer);
}

// ---------------------------------------------------------------------------
// Reallocate
// ---------------------------------------------------------------------------

TEST_CASE("Memory::Reallocate grows a block and preserves its contents", "[memory][reallocate]")
{
    constexpr size64 OriginalSize = 32;
    constexpr size64 GrownSize = 512;

    auto* Pointer = static_cast<uint8*>(Memory::Allocate(OriginalSize));
    REQUIRE(Pointer != nullptr);

    for (size64 Index = 0; Index < OriginalSize; ++Index)
    {
        Pointer[Index] = static_cast<uint8>(Index);
    }

    auto* Grown = static_cast<uint8*>(Memory::Reallocate(Pointer, GrownSize));
    REQUIRE(Grown != nullptr);
    CHECK(Memory::GetAllocationSize(Grown) >= GrownSize);

    bool8 Preserved = true;
    for (size64 Index = 0; Index < OriginalSize; ++Index)
    {
        Preserved = Preserved && (Grown[Index] == static_cast<uint8>(Index));
    }
    CHECK(Preserved);

    Memory::Free(Grown);
}

// ---------------------------------------------------------------------------
// Copy / Move / Set / Zero / Compare
// ---------------------------------------------------------------------------

TEST_CASE("Memory::Copy duplicates bytes and returns the destination", "[memory][copy]")
{
    const uint8 Source[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8 Destination[8] = {};

    void* Result = Memory::Copy(Destination, Source, sizeof(Source));
    CHECK(Result == Destination);
    CHECK(Memory::Compare(Destination, Source, sizeof(Source)) == 0);
}

TEST_CASE("Memory::Move handles overlapping regions", "[memory][move]")
{
    uint8 Buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    // Shift the first four bytes forward by two, overlapping the source range.
    void* Result = Memory::Move(Buffer + 2, Buffer, 4);
    CHECK(Result == Buffer + 2);

    const uint8 Expected[8] = {1, 2, 1, 2, 3, 4, 7, 8};
    CHECK(Memory::Compare(Buffer, Expected, sizeof(Buffer)) == 0);
}

TEST_CASE("Memory::Set fills a buffer with a value", "[memory][set]")
{
    uint8 Buffer[16] = {};

    void* Result = Memory::Set(Buffer, 0x7F, sizeof(Buffer));
    CHECK(Result == Buffer);

    bool8 AllSet = true;
    for (const uint8 Byte : Buffer)
    {
        AllSet = AllSet && (Byte == 0x7F);
    }
    CHECK(AllSet);
}

TEST_CASE("Memory::Zero clears a buffer", "[memory][zero]")
{
    uint8 Buffer[16];
    Memory::Set(Buffer, 0xFF, sizeof(Buffer));

    void* Result = Memory::Zero(Buffer, sizeof(Buffer));
    CHECK(Result == Buffer);

    bool8 AllZero = true;
    for (const uint8 Byte : Buffer)
    {
        AllZero = AllZero && (Byte == 0);
    }
    CHECK(AllZero);
}

TEST_CASE("Memory::Compare orders buffers lexicographically", "[memory][compare]")
{
    const uint8 A[4] = {1, 2, 3, 4};
    const uint8 B[4] = {1, 2, 3, 4};
    const uint8 C[4] = {1, 2, 4, 4};

    CHECK(Memory::Compare(A, B, sizeof(A)) == 0);
    CHECK(Memory::Compare(A, C, sizeof(A)) < 0);
    CHECK(Memory::Compare(C, A, sizeof(A)) > 0);
}

// ---------------------------------------------------------------------------
// IsPowerOfTwo
// ---------------------------------------------------------------------------

TEST_CASE("Memory::IsPowerOfTwo identifies powers of two", "[memory][alignment]")
{
    // Verified at compile time as well as at runtime.
    static_assert(Memory::IsPowerOfTwo(1));
    static_assert(Memory::IsPowerOfTwo(2));
    static_assert(Memory::IsPowerOfTwo(4096));
    static_assert(!Memory::IsPowerOfTwo(0));
    static_assert(!Memory::IsPowerOfTwo(3));

    CHECK(Memory::IsPowerOfTwo(1));
    CHECK(Memory::IsPowerOfTwo(8));
    CHECK(Memory::IsPowerOfTwo(size64(1) << 63));

    CHECK_FALSE(Memory::IsPowerOfTwo(0));
    CHECK_FALSE(Memory::IsPowerOfTwo(6));
    CHECK_FALSE(Memory::IsPowerOfTwo(255));
}

// ---------------------------------------------------------------------------
// AlignUp
// ---------------------------------------------------------------------------

TEST_CASE("Memory::AlignUp rounds up to the next multiple", "[memory][alignment]")
{
    static_assert(Memory::AlignUp(0, 16) == 0);
    static_assert(Memory::AlignUp(16, 16) == 16);
    static_assert(Memory::AlignUp(1, 16) == 16);

    CHECK(Memory::AlignUp(1, 8) == 8);
    CHECK(Memory::AlignUp(8, 8) == 8);
    CHECK(Memory::AlignUp(9, 8) == 16);
    CHECK(Memory::AlignUp(17, 16) == 32);
    CHECK(Memory::AlignUp(100, 64) == 128);
}

// ---------------------------------------------------------------------------
// IsAligned
// ---------------------------------------------------------------------------

TEST_CASE("Memory::IsAligned tests integer alignment", "[memory][alignment]")
{
    static_assert(Memory::IsAligned(size64(0), 16));
    static_assert(Memory::IsAligned(size64(32), 16));

    CHECK(Memory::IsAligned(size64(0), 8));
    CHECK(Memory::IsAligned(size64(64), 32));
    CHECK_FALSE(Memory::IsAligned(size64(1), 8));
    CHECK_FALSE(Memory::IsAligned(size64(20), 16));
}

TEST_CASE("Memory::IsAligned tests pointer alignment", "[memory][alignment]")
{
    alignas(32) uint8 Buffer[64];

    CHECK(Memory::IsAligned(static_cast<const void*>(Buffer), 32));
    CHECK(Memory::IsAligned(static_cast<const void*>(Buffer), 16));
    CHECK_FALSE(Memory::IsAligned(static_cast<const void*>(Buffer + 1), 2));
}

// ---------------------------------------------------------------------------
// Virtual memory
// ---------------------------------------------------------------------------

TEST_CASE("Memory::GetPageSize returns a valid power-of-two page size", "[memory][virtual]")
{
    const size64 PageSize = Memory::GetPageSize();
    CHECK(PageSize > 0);
    CHECK(Memory::IsPowerOfTwo(PageSize));
}

TEST_CASE("Memory reserve/commit/decommit/release lifecycle", "[memory][virtual]")
{
    const size64 PageSize = Memory::GetPageSize();
    const size64 Size = PageSize * 4;

    void* Region = Memory::ReserveVirtual(Size);
    REQUIRE(Region != nullptr);

    const bool Committed = Memory::CommitVirtual(Region, Size);
    REQUIRE(Committed);

    // Committed pages must be readable and writable.
    Memory::Set(Region, 0x5A, Size);
    CHECK(static_cast<uint8*>(Region)[0] == 0x5A);
    CHECK(static_cast<uint8*>(Region)[Size - 1] == 0x5A);

    Memory::DecommitVirtual(Region, Size);
    Memory::ReleaseVirtual(Region, Size);
}

// ---------------------------------------------------------------------------
// New / Delete
// ---------------------------------------------------------------------------

TEST_CASE("Memory::New constructs and Memory::Delete destroys an object", "[memory][new]")
{
    LifetimeProbe::Reset();

    LifetimeProbe* Object = Memory::New<LifetimeProbe>(42);
    REQUIRE(Object != nullptr);
    CHECK(Memory::IsAligned(Object, alignof(LifetimeProbe)));
    CHECK(LifetimeProbe::ConstructCount == 1);
    CHECK(LifetimeProbe::DestructCount == 0);
    CHECK(Object->Value == 42);

    Memory::Delete(Object);
    CHECK(LifetimeProbe::DestructCount == 1);
}

TEST_CASE("Memory::New default-constructs when no arguments are given", "[memory][new]")
{
    LifetimeProbe::Reset();

    LifetimeProbe* Object = Memory::New<LifetimeProbe>();
    REQUIRE(Object != nullptr);
    CHECK(Object->Value == 0);
    CHECK(LifetimeProbe::ConstructCount == 1);

    Memory::Delete(Object);
    CHECK(LifetimeProbe::DestructCount == 1);
}

TEST_CASE("Memory::New respects over-aligned types", "[memory][new]")
{
    OverAligned* Object = Memory::New<OverAligned>();
    REQUIRE(Object != nullptr);
    CHECK(Memory::IsAligned(Object, alignof(OverAligned)));

    Memory::Delete(Object);
}

// ---------------------------------------------------------------------------
// AllocateArray / FreeArray
// ---------------------------------------------------------------------------

TEST_CASE("Memory::AllocateArray returns storage for the requested count", "[memory][array]")
{
    constexpr size64 Count = 32;

    auto* Array = Memory::AllocateArray<uint32>(Count);
    REQUIRE(Array != nullptr);
    CHECK(Memory::IsAligned(Array, alignof(uint32)));
    CHECK(Memory::GetAllocationSize(Array) >= sizeof(uint32) * Count);

    for (size64 Index = 0; Index < Count; ++Index)
    {
        Array[Index] = static_cast<uint32>(Index * Index);
    }

    bool8 Correct = true;
    for (size64 Index = 0; Index < Count; ++Index)
    {
        Correct = Correct && (Array[Index] == static_cast<uint32>(Index * Index));
    }
    CHECK(Correct);

    Memory::FreeArray(Array);
}
