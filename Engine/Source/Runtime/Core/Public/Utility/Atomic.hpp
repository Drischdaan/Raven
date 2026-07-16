// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <atomic>

template <typename T>
using TAtomic = std::atomic<T>;

using FAtomicFlag = std::atomic_flag;
