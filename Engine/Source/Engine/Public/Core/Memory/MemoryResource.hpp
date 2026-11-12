// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <memory_resource>

#include "Core/CoreTypes.hpp"

class RAVEN_API FMemoryResource : public std::pmr::memory_resource
{
private:
    void* do_allocate(size64 Size, size64 Alignment) override;
    void do_deallocate(void* Pointer, size64 Size, size64 Alignment) override;
    bool do_is_equal(const memory_resource& _That) const noexcept override;
};
