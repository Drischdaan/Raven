// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Window_Default.hpp"

#include "CorePlatform.hpp"

#if RAVEN_PLATFORM_WINDOWS
#   define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "GLFW/glfw3native.h"

FWindow_Default::FWindow_Default(const FWindowProperties& InProperties)
    : IWindow(InProperties)
{
    const int32 Width = static_cast<int32>(InProperties.Width);
    const int32 Height = static_cast<int32>(InProperties.Height);
    Window = glfwCreateWindow(Width, Height, "Raven Engine", nullptr, nullptr);
    glfwSetWindowUserPointer(Window, this);

    glfwSetWindowSizeCallback(Window, [](GLFWwindow* Window, const int32 Width, const int32 Height)
    {
        FWindow_Default* This = static_cast<FWindow_Default*>(glfwGetWindowUserPointer(Window));
        This->WindowState.Width = Width;
        This->WindowState.Height = Height;
    });

    glfwSetFramebufferSizeCallback(Window, [](GLFWwindow* Window, const int32 Width, const int32 Height)
    {
        FWindow_Default* This = static_cast<FWindow_Default*>(glfwGetWindowUserPointer(Window));
        This->WindowState.ClientWidth = Width;
        This->WindowState.ClientHeight = Height;
    });

    glfwSetWindowPosCallback(Window, [](GLFWwindow* Window, const int32 X, const int32 Y)
    {
        FWindow_Default* This = static_cast<FWindow_Default*>(glfwGetWindowUserPointer(Window));
        This->WindowState.PositionX = X;
        This->WindowState.PositionY = Y;
    });

    if (InProperties.bShowAfterCreation)
    {
        glfwShowWindow(Window);
    }
}

FWindow_Default::~FWindow_Default()
{
    if (Window != nullptr)
    {
        glfwDestroyWindow(Window);
    }
}

void FWindow_Default::Show()
{
    glfwShowWindow(Window);
}

void FWindow_Default::Hide()
{
    glfwHideWindow(Window);
}

void FWindow_Default::Close()
{
    glfwSetWindowShouldClose(Window, true);
}

void FWindow_Default::Destroy()
{
    glfwDestroyWindow(Window);
    Window = nullptr;
}

bool FWindow_Default::IsClosed() const
{
    return glfwWindowShouldClose(Window);
}

void* FWindow_Default::GetNativeHandle() const
{
#if RAVEN_PLATFORM_WINDOWS
    return glfwGetWin32Window(Window);
#else
    return nullptr;
#endif
}
