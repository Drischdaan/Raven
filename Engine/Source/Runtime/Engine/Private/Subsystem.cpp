// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Subsystem.hpp"

void ISubsystem::Tick(float32 DeltaTime)
{
}

bool8 ISubsystem::IsTickable() const
{
    return false;
}
