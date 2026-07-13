// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include <iostream>
#include <Windows.h>

#include "Error/Result.hpp"

#include "Memory/Memory.hpp"

#include "Profiling/Profiling.hpp"

struct FTexture
{
    uint32 Width = 0;
    uint32 Height = 0;
};

TResult<FTexture> CreateTexture(const uint32 Width, const uint32 Height)
{
    if (Width == 0 || Height == 0)
    {
        return MakeError(RAVEN_DOMAIN_CORE, RAVEN_ERROR_CORE_UNKNOWN);
    }
    return FTexture{.Width = Width, .Height = Height};
}

TResult<int> LoadTexture()
{
    RAVEN_TRY(Texture, CreateTexture(120, 120));
    std::cout << "Texture: " << Texture.Width << "x" << Texture.Height << std::endl;
    return Texture.Width * Texture.Height;
}


RAVEN_DEFINE_PROFILER_ZONE(Setup, Profiler::Colors::Cyan);
RAVEN_DEFINE_PROFILER_ZONE(Test1, Profiler::Colors::Green);
RAVEN_DEFINE_PROFILER_ZONE(Test2, Profiler::Colors::Red);

int main()
{
    RAVEN_PROFILE_ZONE(SetupZone);
    RAVEN_PROFILE_FUNCTION(Profiler::Colors::Red);
    Memory::Initialize();

    float32 Value = 0;
    while (!(GetAsyncKeyState(VK_END) & 0x8000))
    {
        RAVEN_PROFILE_FRAME_MARK();
        if constexpr (true)
        {
            RAVEN_PROFILE_ZONE(Test1Zone);

            if constexpr (true)
            {
                RAVEN_PROFILE_ZONE(Test2Zone);
            }
        }
        RAVEN_PROFILE_PLOT("Test", Value);
        Value++;
    }

    TResult<int> Result = LoadTexture();
    if (!Result)
    {
        std::cout << "Error" << std::endl;
        return 0;
    }

    Memory::Shutdown();
    return Result.GetValue();
}
