// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreCompiler.hpp"
#include "CoreTypes.hpp"

#include "Memory/Containers/String.hpp"

namespace Hash
{
    namespace Internal
    {
        inline constexpr FStringView ProbeName = "double";

        template <typename T>
        [[nodiscard]] consteval FStringView RawSignature()
        {
#if RAVEN_COMPILER_MSVC
            return __FUNCSIG__;
#else
            return __PRETTY_FUNCTION__;
#endif
        }

        [[nodiscard]] consteval size64 PrefixLength()
        {
            return RawSignature<double>().find(ProbeName);
        }

        [[nodiscard]] consteval size64 SuffixLength()
        {
            return RawSignature<double>().size() - PrefixLength() - ProbeName.size();
        }

        template <typename T>
        [[nodiscard]] consteval FStringView QualifiedName()
        {
            constexpr FStringView Raw = RawSignature<T>();
            return Raw.substr(PrefixLength(), Raw.size() - PrefixLength() - SuffixLength());
        }

        [[nodiscard]] consteval bool8 SkipToken(const FStringView Text, size64& Index)
        {
            for (const FStringView Token : {FStringView("struct "), FStringView("class "), FStringView("enum ")})
            {
                if (Text.substr(Index).starts_with(Token))
                {
                    Index += Token.size();
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] consteval uint64 NormalizedFnv1a(const std::string_view Text)
        {
            uint64 Hash = 0xcbf29ce484222325ull;
            for (size64 Index = 0; Index < Text.size();)
            {
                if (SkipToken(Text, Index))
                {
                    continue;
                }
                if (Text[Index] == ' ')
                {
                    ++Index;
                    continue;
                }
                Hash ^= static_cast<uint8>(Text[Index++]);
                Hash *= 0x00000100000001b3ull;
            }
            return Hash;
        }

        [[nodiscard]] consteval bool IsNameable(const std::string_view Name)
        {
            return Name.find("lambda") == std::string_view::npos && Name.find("anonymous") == std::string_view::npos && Name.find("unnamed") == std::string_view::npos;
        }
    }

    template <typename TType>
    [[nodiscard]] consteval std::string_view TypeName()
    {
        return Internal::QualifiedName<TType>();
    }

    template <typename TType>
    [[nodiscard]] consteval uint64 TypeNameHash()
    {
        constexpr std::string_view Name = Internal::QualifiedName<TType>();
        static_assert(Internal::IsNameable(Name), "Fragments must be named types at namespace scope — lambdas and anonymous-namespace types have no stable identity");
        return Internal::NormalizedFnv1a(Name);
    }
}

static_assert(Hash::TypeName<double>() == "double");
