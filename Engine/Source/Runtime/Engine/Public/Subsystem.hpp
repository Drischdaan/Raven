// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "Error/Result.hpp"

class ENGINE_API ISubsystem
{
public:
    ISubsystem() = default;
    ISubsystem(const ISubsystem&) = delete;
    virtual ~ISubsystem() = default;

    ISubsystem& operator=(const ISubsystem&) = delete;

public:
    [[nodiscard]] virtual TResult<void> Initialize() = 0;
    virtual void Shutdown() = 0;

    virtual void Tick(float32 DeltaTime);
    [[nodiscard]] virtual bool8 IsTickable() const;
};
