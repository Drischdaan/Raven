// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Engine.hpp"

#include <ostream>

#if RAVEN_PLATFORM_WINDOWS
#   include <Windows.h>
#endif

#include <iostream>

#include "EngineProfilerZones.hpp"
#include "Subsystem.hpp"

#include "JobSystem/JobSubsystem.hpp"

#include "Profiling/Profiling.hpp"

namespace
{
    FEngine* GEngineInstance = nullptr;
}

TResult<void> FEngine::Initialize()
{
    RAVEN_CORE_ASSERT(!bIsInitialized, "Engine already initialized!");

    RAVEN_PROFILE_FUNCTION();

    AddSubsystem<FJobSubsystem>();

    for (const FSubsystemEntry& Entry : Subsystems)
    {
        RAVEN_TRY_VOID(Entry.Subsystem->Initialize())
    }

    bIsInitialized = true;
    return {};
}

RAVEN_DEFINE_PROFILER_ZONE(Test, Profiler::Colors::Yellow);

static void ForkJoin(FJobSubsystem* Jobs, int Depth, std::atomic<int>& LeafCount)
{
    if (Depth == 0)
    {
        LeafCount.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    FJobCounter Children;
    Jobs->Kick([&Jobs, Depth, &LeafCount] { ForkJoin(Jobs, Depth - 1, LeafCount); }, &Children);
    Jobs->Kick([&Jobs, Depth, &LeafCount] { ForkJoin(Jobs, Depth - 1, LeafCount); }, &Children);

    // Parks THIS fiber (pulls a fresh one to run the children), resumes when both finish.                                                                                                                                                                                                                        
    Jobs->WaitForCounter(Children, 0);
}

void FEngine::Run()
{
    RAVEN_CORE_ASSERT(bIsInitialized, "Engine not initialized!");
    RAVEN_CORE_ASSERT(!bIsRunning, "Engine already running!");
    bIsRunning = true;

    bool8 bTest = false;
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

            if ((GetAsyncKeyState(VK_END) & 0x8000) && !bTest)
            {
                bTest = true;
                std::cout << "Calling jobs" << std::endl;
                FJobSubsystem* JobSubsystem = GetSubsystem<FJobSubsystem>();
                FJobCounter Frame;
                JobSubsystem->Kick([]
                {
                    const FString TestZoneText = "Physics";
                    RAVEN_PROFILE_ZONE_DYNAMIC(TestZone, TestZoneText.data(), TestZoneText.size());
                    uint64 Test = 0;
                    for (int32 Index = 0; Index < 100'000; ++Index)
                    {
                        Test += Index;
                    }
                }, &Frame);
                JobSubsystem->Kick([]
                {
                    const FString TestZoneText = "Animate";
                    RAVEN_PROFILE_ZONE_DYNAMIC(TestZone, TestZoneText.data(), TestZoneText.size());
                    uint64 Test = 0;
                    for (int32 Index = 0; Index < 100'000; ++Index)
                    {
                        Test += Index;
                    }
                }, &Frame, EJobPriority::High);

                FJobCounter PostFrame;
                JobSubsystem->KickAfter(Frame, 0, []
                {
                    const FString TestZoneText = "BuildDrawList";
                    RAVEN_PROFILE_ZONE_DYNAMIC(TestZone, TestZoneText.data(), TestZoneText.size());
                    uint64 Test = 0;
                    for (int32 Index = 0; Index < 100'000; ++Index)
                    {
                        Test += Index;
                    }
                }, &PostFrame);
                JobSubsystem->KickAfter(Frame, 0, []
                {
                    const FString TestZoneText = "Render";
                    RAVEN_PROFILE_ZONE_DYNAMIC(TestZone, TestZoneText.data(), TestZoneText.size());
                    uint64 Test = 0;
                    for (int32 Index = 0; Index < 100'000; ++Index)
                    {
                        Test += Index;
                    }
                }, &PostFrame);

                JobSubsystem->ParallelFor(100'000, [](const size64 Begin, const size64 End)
                {
                    const FString TestZoneText = "Transform";
                    RAVEN_PROFILE_ZONE_DYNAMIC(TestZone, TestZoneText.data(), TestZoneText.size());
                    for (size64 Index = Begin; Index < End; ++Index)
                    {
                        /* transform */
                    }
                });

                JobSubsystem->ParallelFor(100'000, [](const size64 Begin, const size64 End)
                {
                    const FString TestZoneText = "Transform";
                    RAVEN_PROFILE_ZONE_DYNAMIC(TestZone, TestZoneText.data(), TestZoneText.size());
                    for (size64 Index = Begin; Index < End; ++Index)
                    {
                        /* transform */
                    }
                });

                JobSubsystem->ParallelFor(100'000, [](const size64 Begin, const size64 End)
                {
                    const FString TestZoneText = "Transform";
                    RAVEN_PROFILE_ZONE_DYNAMIC(TestZone, TestZoneText.data(), TestZoneText.size());
                    for (size64 Index = Begin; Index < End; ++Index)
                    {
                        /* transform */
                    }
                });

                JobSubsystem->ParallelFor(100'000, [](const size64 Begin, const size64 End)
                {
                    const FString TestZoneText = "Transform";
                    RAVEN_PROFILE_ZONE_DYNAMIC(TestZone, TestZoneText.data(), TestZoneText.size());
                    for (size64 Index = Begin; Index < End; ++Index)
                    {
                        /* transform */
                    }
                });

                JobSubsystem->WaitForCounter(PostFrame);

                FJobCounter Root;
                std::atomic<int> LeafCount = 0;
                JobSubsystem->Kick([&] { ForkJoin(JobSubsystem, /*Depth*/ 6, LeafCount); }, &Root);
                JobSubsystem->WaitForCounter(Root, 0);

                std::cout << "Jobs End" << std::endl;
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
