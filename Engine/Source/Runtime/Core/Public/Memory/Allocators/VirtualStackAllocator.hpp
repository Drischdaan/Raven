// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "LinearAllocator.hpp"

/**
 * @class FVirtualStackAllocator
 * @brief A specialized allocator built upon `FLinearAllocator` with stack-based allocation and rollback capabilities. Primarily used with the job system.
 *
 * Should presumably be used with the job system or one per temporary thread for scratch space.
 */
class CORE_API FVirtualStackAllocator final : public FLinearAllocator
{
public:
    using FLinearAllocator::FLinearAllocator;

    class FScope
    {
    public:
        explicit FScope(FVirtualStackAllocator& InAllocator) noexcept;
        FScope(const FScope&) = delete;
        ~FScope() noexcept;

        FScope& operator=(const FScope&) = delete;

    protected:
        FVirtualStackAllocator& Allocator;
        size64 Marker;
    };
};
