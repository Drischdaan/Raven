// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Engine.hpp"

#include "Memory/Memory.hpp"

void Shutdown()
{
    FEngine* Engine = GetEngine();
    if (Engine != nullptr)
    {
        Engine->Shutdown();
        DestroyEngine();
    }
    Memory::Shutdown();
}

int main()
{
    Memory::Initialize();
    FEngine* Engine = GetEngine();
    if (const TResult<void> InitializationResult = Engine->Initialize(); !InitializationResult)
    {
        Shutdown();
        return 1;
    }
    Engine->Run();
    Shutdown();
    return 0;
}
