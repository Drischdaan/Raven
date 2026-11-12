// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <string>
#include <format>
#include <iterator>

using TChar = char;
using FString = std::pmr::basic_string<TChar>;
using FStringView = std::basic_string_view<TChar>;

template <typename... TArguments>
using TFormatString = std::basic_format_string<TChar, std::type_identity_t<TArguments>...>;

using TWideChar = wchar_t;
using FWideString = std::pmr::basic_string<TWideChar>;
using FWideStringView = std::basic_string_view<TWideChar>;

template <typename... TArguments>
using TWideFormatString = std::basic_format_string<TWideChar, std::type_identity_t<TArguments>...>;

namespace String
{
    template <typename TString, typename TFormat, typename... TArguments>
    [[nodiscard]] TString Format(TFormat FormatString, TArguments&&... Arguments)
    {
        TString Result;
        Result.reserve(std::formatted_size(FormatString, Arguments...));
        std::format_to(std::back_inserter(Result), FormatString, std::forward<TArguments>(Arguments)...);
        return Result;
    }

    template <typename... TArguments>
    [[nodiscard]] FString Format(TFormatString<TArguments...> FormatString, TArguments&&... Arguments)
    {
        return Format<FString, TFormatString<TArguments...>>(FormatString, std::forward<TArguments>(Arguments)...);
    }

    template <typename... TArguments>
    [[nodiscard]] FWideString Format(TWideFormatString<TArguments...> FormatString, TArguments&&... Arguments)
    {
        return Format<FWideString, TWideFormatString<TArguments...>>(FormatString, std::forward<TArguments>(Arguments)...);
    }
}
