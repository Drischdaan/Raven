// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <map>
#include <unordered_map>

template <typename TKey, typename TValue, typename TComparer = std::less<TKey>> requires std::is_object_v<TKey>
using TMap = std::pmr::map<TKey, TValue, TComparer>;

template <typename TKey, typename TValue, typename THasher = std::hash<TKey>, typename TComparer = std::equal_to<TKey>> requires std::is_object_v<TKey>
using TUnorderedMap = std::pmr::unordered_map<TKey, TValue, THasher, TComparer>;
