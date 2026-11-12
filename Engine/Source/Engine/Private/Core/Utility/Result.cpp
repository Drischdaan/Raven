// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Core/Utility/Result.hpp"

#include <Windows.h>

std::unexpected<FRavenError> MakeRavenError(const FStringView Message, std::source_location Location)
{
    return std::unexpected(FRavenError{
        .PlatformCode = 0,
        .Location = Location,
        .Message = FString(Message),
    });
}

std::unexpected<FRavenError> MakeRavenPlatformError(const FStringView Message, std::source_location Location)
{
    return std::unexpected(FRavenError{
        .PlatformCode = GetLastError(),
        .Location = Location,
        .Message = FString(Message),
    });
}
