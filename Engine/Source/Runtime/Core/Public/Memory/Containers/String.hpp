// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <format>
#include <string>

using TChar = char;
using FString = std::pmr::string;
using FStringView = std::string_view;

template <typename T, typename... TArguments>
using TFormatString = std::format_string<T, TArguments...>;

using TWideChar = wchar_t;
using FWideString = std::pmr::wstring;
using FWideStringView = std::wstring_view;

template <typename T, typename... TArguments>
using TFormatWideString = std::wformat_string<T, TArguments...>;
