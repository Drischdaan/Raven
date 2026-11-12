// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <map>
#include <unordered_map>

template <typename TKey, typename TValue, typename TComparer = std::less<TKey>>
using TMap = std::pmr::map<TKey, TValue, TComparer>;

template <typename TKey, typename TValue, typename TComparer = std::less<TKey>>
using TMultiMap = std::pmr::multimap<TKey, TValue, TComparer>;

template <typename TKey, typename TValue, typename THasher = std::hash<TKey>, typename TComparer = std::equal_to<TKey>>
using TUnorderedMap = std::pmr::unordered_map<TKey, TValue, THasher, TComparer>;

template <typename TKey, typename TValue, typename THasher = std::hash<TKey>, typename TComparer = std::equal_to<TKey>>
using TUnorderedMultiMap = std::pmr::unordered_multimap<TKey, TValue, THasher, TComparer>;
