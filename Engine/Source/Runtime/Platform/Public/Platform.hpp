// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once
#include "Window.hpp"

class PLATFORM_API FPlatform
{
public:
    static void Initialize();
    static void Update();
    static void Shutdown();

    [[nodiscard]] static IWindow* CreateWindow(const FWindowProperties& Properties);

private:
};
