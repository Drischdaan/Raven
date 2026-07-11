// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "Memory/SmartPointers.hpp"

struct FWindowProperties;
class IWindow;

class IWindowManager
{
public:
    IWindowManager() = default;
    ~IWindowManager() = default;

public:
    [[nodiscard]] TSharedPtr<IWindow> CreateWindow(const FWindowProperties& Properties);

private:
};
