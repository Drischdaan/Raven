// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"

#include "Utility/Atomic.hpp"

class ENGINE_API FSpinLock
{
public:
    void Lock();
    [[nodiscard]] bool8 TryLock();
    void Unlock();

private:
    TAtomic<bool8> Flag = false;
};
