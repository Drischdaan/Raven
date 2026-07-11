// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Error/Result.hpp"

#include <utility>

#include "snitch/snitch.hpp"

namespace
{
    // Distinguishes move-construction from copy-construction so the value category
    // forwarded by GetValue can be observed.
    struct MoveTracker
    {
        static inline int32 MoveCount = 0;
        static inline int32 CopyCount = 0;

        int32 Value = 0;

        MoveTracker() noexcept = default;
        explicit MoveTracker(const int32 InValue) noexcept : Value(InValue) {}

        MoveTracker(const MoveTracker& Other) noexcept : Value(Other.Value) { ++CopyCount; }
        MoveTracker(MoveTracker&& Other) noexcept : Value(Other.Value) { ++MoveCount; }

        MoveTracker& operator=(const MoveTracker& Other) noexcept
        {
            Value = Other.Value;
            ++CopyCount;
            return *this;
        }

        MoveTracker& operator=(MoveTracker&& Other) noexcept
        {
            Value = Other.Value;
            ++MoveCount;
            return *this;
        }

        static void Reset() noexcept
        {
            MoveCount = 0;
            CopyCount = 0;
        }
    };

    // Halves an even number; an odd number produces an error carrying the offending
    // value in the payload so the propagated error can be identified.
    TResult<int32> Halve(const int32 Value)
    {
        if (Value % 2 != 0)
        {
            return MakeError(RAVEN_DOMAIN_CORE, RAVEN_ERROR_CORE_UNKNOWN, static_cast<uint32>(Value));
        }
        return Value / 2;
    }

    // Chains two fallible calls with RAVEN_TRY; the first error short-circuits.
    TResult<int32> HalveTwice(const int32 Value)
    {
        RAVEN_TRY(First, Halve(Value));
        RAVEN_TRY(Second, Halve(First));
        return Second;
    }

    // Void-returning step; failure carries Code in the payload.
    TResult<void> Step(const bool8 Fail, const uint32 Code)
    {
        if (Fail)
        {
            return MakeError(RAVEN_DOMAIN_CORE, RAVEN_ERROR_CORE_UNKNOWN, Code);
        }
        return TResult<void>{};
    }

    // Runs two void steps with RAVEN_TRY_VOID, recording how far execution reached.
    TResult<void> RunSteps(const bool8 FailSecond, int32& Reached)
    {
        RAVEN_TRY_VOID(Step(false, 1));
        Reached = 1;
        RAVEN_TRY_VOID(Step(FailSecond, 2));
        Reached = 2;
        return TResult<void>{};
    }
}

// ---------------------------------------------------------------------------
// FError
// ---------------------------------------------------------------------------

TEST_CASE("FError defaults to the unknown domain and code", "[result][error]")
{
    const FError Error{};

    CHECK(Error.Domain == RAVEN_DOMAIN_UNKNOWN);
    CHECK(Error.Code == RAVEN_ERROR_CORE_UNKNOWN);
    CHECK(Error.Payload == 0);
}

// ---------------------------------------------------------------------------
// MakeError
// ---------------------------------------------------------------------------

TEST_CASE("MakeError populates domain, code and payload", "[result][error]")
{
    const std::unexpected<FError> Error = MakeError(RAVEN_DOMAIN_CORE, 5, 99);

    CHECK(Error.error().Domain == RAVEN_DOMAIN_CORE);
    CHECK(Error.error().Code == 5);
    CHECK(Error.error().Payload == 99);
}

TEST_CASE("MakeError defaults the payload to zero", "[result][error]")
{
    const std::unexpected<FError> Error = MakeError(RAVEN_DOMAIN_CORE, 7);

    CHECK(Error.error().Code == 7);
    CHECK(Error.error().Payload == 0);
}

// ---------------------------------------------------------------------------
// Value / error state
// ---------------------------------------------------------------------------

TEST_CASE("TResult holding a value reports HasValue", "[result][state]")
{
    const TResult<int32> Result = 42;

    CHECK(Result.HasValue());
    CHECK_FALSE(Result.HasError());
}

TEST_CASE("TResult holding an error reports HasError", "[result][state]")
{
    const TResult<int32> Result = MakeError(RAVEN_DOMAIN_CORE, RAVEN_ERROR_CORE_UNKNOWN);

    CHECK(Result.HasError());
    CHECK_FALSE(Result.HasValue());
}

// ---------------------------------------------------------------------------
// GetValue
// ---------------------------------------------------------------------------

TEST_CASE("GetValue returns the stored value", "[result][getvalue]")
{
    const TResult<int32> Result = 7;

    CHECK(Result.GetValue() == 7);
}

TEST_CASE("GetValue yields a mutable reference on a non-const result", "[result][getvalue]")
{
    TResult<int32> Result = 1;

    Result.GetValue() = 100;
    CHECK(Result.GetValue() == 100);
}

TEST_CASE("GetValue is callable on a const result", "[result][getvalue]")
{
    const TResult<int32> Result = 55;

    // Compiles only because the deducing-this overload forwards const-ness.
    const int32& Value = Result.GetValue();
    CHECK(Value == 55);
}

TEST_CASE("GetValue copies from an lvalue result", "[result][getvalue]")
{
    TResult<MoveTracker> Result{MoveTracker{9}};

    MoveTracker::Reset();
    const MoveTracker Copied = Result.GetValue();

    CHECK(Copied.Value == 9);
    CHECK(MoveTracker::CopyCount == 1);
    CHECK(MoveTracker::MoveCount == 0);
}

TEST_CASE("GetValue moves from an rvalue result", "[result][getvalue]")
{
    TResult<MoveTracker> Result{MoveTracker{7}};

    MoveTracker::Reset();
    const MoveTracker Moved = std::move(Result).GetValue();

    CHECK(Moved.Value == 7);
    CHECK(MoveTracker::MoveCount == 1);
    CHECK(MoveTracker::CopyCount == 0);
}

// ---------------------------------------------------------------------------
// ValueOr
// ---------------------------------------------------------------------------

TEST_CASE("ValueOr returns the stored value when present", "[result][valueor]")
{
    const TResult<int32> Result = 5;

    CHECK(Result.ValueOr(99) == 5);
}

TEST_CASE("ValueOr returns the fallback on an error", "[result][valueor]")
{
    const TResult<int32> Result = MakeError(RAVEN_DOMAIN_CORE, RAVEN_ERROR_CORE_UNKNOWN);

    CHECK(Result.ValueOr(99) == 99);
}

TEST_CASE("ValueOr does not overwrite the stored value", "[result][valueor]")
{
    const TResult<int32> Result = 3;

    (void)Result.ValueOr(99);
    CHECK(Result.GetValue() == 3);
}

// ---------------------------------------------------------------------------
// GetError
// ---------------------------------------------------------------------------

TEST_CASE("GetError exposes the underlying error", "[result][geterror]")
{
    const TResult<int32> Result = MakeError(RAVEN_DOMAIN_CORE, 8, 123);

    REQUIRE(Result.HasError());
    CHECK(Result.GetError().Domain == RAVEN_DOMAIN_CORE);
    CHECK(Result.GetError().Code == 8);
    CHECK(Result.GetError().Payload == 123);
}

// ---------------------------------------------------------------------------
// TResult<void>
// ---------------------------------------------------------------------------

TEST_CASE("A default-constructed TResult<void> holds a value", "[result][void]")
{
    const TResult<void> Result{};

    CHECK(Result.HasValue());
    CHECK_FALSE(Result.HasError());
}

TEST_CASE("TResult<void> can carry an error", "[result][void]")
{
    const TResult<void> Result = MakeError(RAVEN_DOMAIN_CORE, 3, 77);

    REQUIRE(Result.HasError());
    CHECK(Result.GetError().Code == 3);
    CHECK(Result.GetError().Payload == 77);
}

// ---------------------------------------------------------------------------
// RAVEN_TRY
// ---------------------------------------------------------------------------

TEST_CASE("RAVEN_TRY unwraps values and forwards the result", "[result][try]")
{
    const TResult<int32> Result = HalveTwice(8);

    REQUIRE(Result.HasValue());
    CHECK(Result.GetValue() == 2);
}

TEST_CASE("RAVEN_TRY short-circuits and propagates the error", "[result][try]")
{
    // Halve(6) -> 3, then Halve(3) fails carrying 3 in the payload.
    const TResult<int32> Result = HalveTwice(6);

    REQUIRE(Result.HasError());
    CHECK(Result.GetError().Payload == 3);
}

TEST_CASE("RAVEN_TRY propagates an error from the first call", "[result][try]")
{
    const TResult<int32> Result = HalveTwice(3);

    REQUIRE(Result.HasError());
    CHECK(Result.GetError().Payload == 3);
}

// ---------------------------------------------------------------------------
// RAVEN_TRY_VOID
// ---------------------------------------------------------------------------

TEST_CASE("RAVEN_TRY_VOID runs every step on success", "[result][try][void]")
{
    int32 Reached = 0;
    const TResult<void> Result = RunSteps(false, Reached);

    CHECK(Result.HasValue());
    CHECK(Reached == 2);
}

TEST_CASE("RAVEN_TRY_VOID short-circuits on failure", "[result][try][void]")
{
    int32 Reached = 0;
    const TResult<void> Result = RunSteps(true, Reached);

    REQUIRE(Result.HasError());
    CHECK(Result.GetError().Payload == 2);
    CHECK(Reached == 1);
}
