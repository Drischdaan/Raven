// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Utility/Hashing.hpp"

#include "snitch/snitch.hpp"

// Named-namespace types so their qualified names are stable and nameable across
// compilers (anonymous-namespace types would fold into "anonymous" and fail
// IsNameable).
namespace HashingTests
{
    struct Widget
    {
        int32 Value = 0;
    };

    struct Gadget
    {
        int32 Value = 0;
    };

    enum class Color
    {
        Red,
        Green,
    };
}

// ---------------------------------------------------------------------------
// TypeName
// ---------------------------------------------------------------------------

TEST_CASE("TypeName resolves fundamental type spellings", "[hashing][typename]")
{
    // Fundamental spellings are identical across supported compilers.
    CHECK(Hash::TypeName<double>() == "double");
    CHECK(Hash::TypeName<float>() == "float");
    CHECK(Hash::TypeName<int>() == "int");
    CHECK(Hash::TypeName<char>() == "char");
    CHECK(Hash::TypeName<bool>() == "bool");
}

TEST_CASE("TypeName includes the enclosing namespace for user types", "[hashing][typename]")
{
    // The keyword prefix ("struct "/"class ") is compiler-dependent and left
    // intact here, but the qualified name is always present.
    CHECK(Hash::TypeName<HashingTests::Widget>().find("HashingTests::Widget") != std::string_view::npos);
}

TEST_CASE("TypeName distinguishes distinct user types", "[hashing][typename]")
{
    CHECK(Hash::TypeName<HashingTests::Widget>() != Hash::TypeName<HashingTests::Gadget>());
}

// ---------------------------------------------------------------------------
// NormalizedFnv1a
// ---------------------------------------------------------------------------

TEST_CASE("NormalizedFnv1a of an empty string is the FNV offset basis", "[hashing][fnv1a]")
{
    CHECK(Hash::Internal::NormalizedFnv1a("") == 0xcbf29ce484222325ull);
}

TEST_CASE("NormalizedFnv1a matches the canonical FNV-1a vectors", "[hashing][fnv1a]")
{
    // Standard 64-bit FNV-1a test vectors; inputs carry no whitespace or
    // keywords so normalization is a no-op here.
    CHECK(Hash::Internal::NormalizedFnv1a("a") == 0xaf63dc4c8601ec8cull);
    CHECK(Hash::Internal::NormalizedFnv1a("foobar") == 0x85944171f73967e8ull);
}

TEST_CASE("NormalizedFnv1a strips the struct, class and enum keywords", "[hashing][fnv1a]")
{
    CHECK(Hash::Internal::NormalizedFnv1a("struct Foo") == Hash::Internal::NormalizedFnv1a("Foo"));
    CHECK(Hash::Internal::NormalizedFnv1a("class Foo") == Hash::Internal::NormalizedFnv1a("Foo"));
    CHECK(Hash::Internal::NormalizedFnv1a("enum Foo") == Hash::Internal::NormalizedFnv1a("Foo"));
}

TEST_CASE("NormalizedFnv1a strips keywords wherever they appear", "[hashing][fnv1a]")
{
    // Elaborated-type keywords nested inside template arguments must also fold
    // away so equivalent spellings collide.
    CHECK(Hash::Internal::NormalizedFnv1a("TArray<struct Foo>") == Hash::Internal::NormalizedFnv1a("TArray<Foo>"));
}

TEST_CASE("NormalizedFnv1a ignores whitespace", "[hashing][fnv1a]")
{
    CHECK(Hash::Internal::NormalizedFnv1a("Foo Bar") == Hash::Internal::NormalizedFnv1a("FooBar"));
}

TEST_CASE("NormalizedFnv1a only strips a keyword followed by a space", "[hashing][fnv1a]")
{
    // "struct " is a token including its trailing space; "structFoo" is a plain
    // identifier and must not be normalized to "Foo".
    CHECK(Hash::Internal::NormalizedFnv1a("structFoo") != Hash::Internal::NormalizedFnv1a("Foo"));
}

// ---------------------------------------------------------------------------
// IsNameable
// ---------------------------------------------------------------------------

TEST_CASE("IsNameable accepts ordinary qualified names", "[hashing][nameable]")
{
    CHECK(Hash::Internal::IsNameable("Foo"));
    CHECK(Hash::Internal::IsNameable("HashingTests::Widget"));
}

TEST_CASE("IsNameable rejects lambda, anonymous and unnamed spellings", "[hashing][nameable]")
{
    CHECK_FALSE(Hash::Internal::IsNameable("<lambda_1>"));
    CHECK_FALSE(Hash::Internal::IsNameable("`anonymous namespace'::Foo"));
    CHECK_FALSE(Hash::Internal::IsNameable("<unnamed-type-Value>"));
}

// ---------------------------------------------------------------------------
// TypeNameHash
// ---------------------------------------------------------------------------

TEST_CASE("TypeNameHash is a non-zero, deterministic value", "[hashing][typenamehash]")
{
    CHECK(Hash::TypeNameHash<double>() != 0);
    CHECK(Hash::TypeNameHash<double>() == Hash::TypeNameHash<double>());
}

TEST_CASE("TypeNameHash distinguishes distinct types", "[hashing][typenamehash]")
{
    CHECK(Hash::TypeNameHash<int>() != Hash::TypeNameHash<double>());
    CHECK(Hash::TypeNameHash<HashingTests::Widget>() != Hash::TypeNameHash<HashingTests::Gadget>());
}

TEST_CASE("TypeNameHash is the normalized FNV-1a of the type name", "[hashing][typenamehash]")
{
    // The hash must be independent of the compiler's keyword prefix, so it
    // equals the normalized hash of the raw qualified spelling.
    CHECK(Hash::TypeNameHash<HashingTests::Widget>() == Hash::Internal::NormalizedFnv1a(Hash::TypeName<HashingTests::Widget>()));
    CHECK(Hash::TypeNameHash<HashingTests::Color>() == Hash::Internal::NormalizedFnv1a(Hash::TypeName<HashingTests::Color>()));
}
