// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Engine.hpp"
#include "Core/Memory/Containers/String.hpp"

extern "C" {
__declspec(dllexport) extern const uint32 D3D12SDKVersion = 619;
}

extern "C" {
__declspec(dllexport) extern const TChar* D3D12SDKPath = ".\\";
}

int main()
{
    StartupEngine();
    return 0;
}
