// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include <iostream>

#include "Error/Result.hpp"

#include "Memory/Memory.hpp"

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

int main()
{
    Memory::Initialize();

    TResult<int> Result = LoadTexture();
    if (!Result)
    {
        std::cout << "Error" << std::endl;
        return 0;
    }

    Memory::Shutdown();
    return Result.GetValue();
}
