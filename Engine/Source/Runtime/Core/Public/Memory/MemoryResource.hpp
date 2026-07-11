// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"

#include <memory_resource>

class FLinearAllocator;

class FMemoryResource : public std::pmr::memory_resource
{
private:
    void* do_allocate(size64 Size, size64 Alignment) override;
    void do_deallocate(void* Pointer, size64 Size, size64 Alignment) override;
    bool8 do_is_equal(const memory_resource& Other) const noexcept override;

public:
    static FMemoryResource* Get();
};

class FLinearMemoryResource : public std::pmr::memory_resource
{
public:
    explicit FLinearMemoryResource(FLinearAllocator& InLinearAllocator);

private:
    void* do_allocate(size64 Size, size64 Alignment) override;
    void do_deallocate(void* Pointer, size64 Size, size64 Alignment) override;
    bool8 do_is_equal(const memory_resource& Other) const noexcept override;

protected:
    FLinearAllocator& LinearAllocator;
};
