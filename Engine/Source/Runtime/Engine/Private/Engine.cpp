// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Engine.hpp"

#include <ostream>

#if RAVEN_PLATFORM_WINDOWS
#   include <Windows.h>
#endif

#include "EngineProfilerZones.hpp"
#include "Subsystem.hpp"
#include "Profiling/Profiling.hpp"

namespace
{
    FEngine* GEngineInstance = nullptr;
}

TResult<void> FEngine::Initialize()
{
    RAVEN_CORE_ASSERT(!bIsInitialized, "Engine already initialized!");

    RAVEN_PROFILE_FUNCTION();

    for (const FSubsystemEntry& Entry : Subsystems)
    {
        RAVEN_TRY_VOID(Entry.Subsystem->Initialize())
    }

    bIsInitialized = true;
    return {};
}

void FEngine::Run()
{
    RAVEN_CORE_ASSERT(bIsInitialized, "Engine not initialized!");
    RAVEN_CORE_ASSERT(!bIsRunning, "Engine already running!");
    bIsRunning = true;

    while (!bIsExitRequested)
    {
        RAVEN_PROFILE_FRAME_MARK();
        {
            RAVEN_PROFILE_ZONE(EngineProfilerZones::SubsystemTickZone);
            for (const FSubsystemEntry& Entry : Subsystems)
            {
                if (Entry.Subsystem->IsTickable())
                {
                    RAVEN_PROFILE_ZONE_DYNAMIC(EngineProfilerZones::SubsystemTickZone, Entry.Name.data(), Entry.Name.size());
                    Entry.Subsystem->Tick(0.0f); // TODO: Give delta time
                }
            }

            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) // TODO: Remove. Temporary manual way to stop the engine
            {
                RequestExit();
            }
        }
    }
    bIsRunning = false;
}

void FEngine::Shutdown()
{
    RAVEN_CORE_ASSERT(bIsInitialized, "Engine not initialized!");
    RAVEN_PROFILE_FUNCTION();

    SubsystemLookupMap.clear();
    for (const FSubsystemEntry& Entry : Subsystems)
    {
        Memory::Delete(Entry.Subsystem);
    }
    Subsystems.clear();

    bIsInitialized = false;
}

void FEngine::RequestExit() noexcept
{
    bIsExitRequested = true;
}

bool8 FEngine::IsExitRequested() const noexcept
{
    return bIsExitRequested;
}

FEngine* GetEngine()
{
    if (GEngineInstance == nullptr)
    {
        GEngineInstance = Memory::New<FEngine>();
    }
    return GEngineInstance;
}

void DestroyEngine()
{
    RAVEN_CORE_ASSERT(GEngineInstance != nullptr, "Engine not initialized!");
    Memory::Delete(GEngineInstance);
    GEngineInstance = nullptr;
}
