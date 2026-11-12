// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <queue>
#include <deque>

#include "Vector.hpp"

template <typename T> requires std::is_object_v<T>
using TDeque = std::pmr::deque<T>;

template <typename T, typename TContainer = TDeque<T>> requires std::is_object_v<T>
using TQueue = std::queue<T, TContainer>;

template <typename T, typename TContainer = TVector<T>, typename TComparer = std::less<T>>
using TPriorityQueue = std::priority_queue<T, TContainer, TComparer>;
