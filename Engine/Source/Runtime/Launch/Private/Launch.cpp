// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include <iostream>

#include "Memory/Handle.hpp"
#include "Memory/Memory.hpp"
#include "Memory/Containers/SparseHandlePool.hpp"

struct FTexture
{
    uint32 Width = 0;
    uint32 Height = 0;
};

using FTextureHandle = THandle<FTexture>;

int main()
{
    Memory::Initialize();

    TSparseHandlePool<FTexture> TexturePool;
    const FTextureHandle Handle1 = TexturePool.Create(FTexture{.Width = 1280, .Height = 720});
    const FTextureHandle Handle2 = TexturePool.Create(FTexture{.Width = 400, .Height = 400});

    std::cout << "Handle1: " << TexturePool.Resolve(Handle1).Width << "x" << TexturePool.Resolve(Handle1).Height << std::endl;
    std::cout << "Handle2: " << TexturePool.Resolve(Handle2).Width << "x" << TexturePool.Resolve(Handle2).Height << std::endl;

    TexturePool.Destroy(Handle1);
    TexturePool.Destroy(Handle2);

    Memory::Shutdown();
    return 0;
}
