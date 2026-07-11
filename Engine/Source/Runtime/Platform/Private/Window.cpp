// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Window.hpp"

IWindow::IWindow(const FWindowProperties& InProperties)
{
    WindowState.Width = InProperties.Width;
    WindowState.Height = InProperties.Height;
    WindowState.PositionX = InProperties.StartPositionX;
    WindowState.PositionY = InProperties.StartPositionY;
}

const FWindowState& IWindow::GetWindowState() const
{
    return WindowState;
}
