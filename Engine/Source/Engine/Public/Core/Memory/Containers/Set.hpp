// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <set>
#include <unordered_set>

template <typename TKey, typename TComparer = std::less<TKey>>
using TSet = std::pmr::set<TKey, TComparer>;

template <typename TKey, typename TComparer = std::less<TKey>>
using TMultiSet = std::pmr::multiset<TKey, TComparer>;

template <typename TKey, typename THasher = std::hash<TKey>, typename TComparer = std::equal_to<TKey>>
using TUnorderedSet = std::pmr::unordered_set<TKey, THasher, TComparer>;

template <typename TKey, typename THasher = std::hash<TKey>, typename TComparer = std::equal_to<TKey>>
using TUnorderedMultiSet = std::pmr::unordered_multiset<TKey, THasher, TComparer>;
