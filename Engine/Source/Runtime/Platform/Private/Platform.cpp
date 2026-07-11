// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Platform.hpp"

#include "Window_Default.hpp"

#include "GLFW/glfw3.h"

void FPlatform::Initialize()
{
    glfwInit();
}

void FPlatform::Update()
{
    glfwPollEvents();
}

void FPlatform::Shutdown()
{
    glfwTerminate();
}

IWindow* FPlatform::CreateWindow(const FWindowProperties& Properties)
{
    return new FWindow_Default(Properties);
}
