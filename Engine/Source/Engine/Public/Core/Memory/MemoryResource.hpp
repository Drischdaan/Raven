// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <memory_resource>

#include "Allocators/VirtualMemoryArena.hpp"
#include "Core/CoreTypes.hpp"

class RAVEN_API FMemoryResource : public std::pmr::memory_resource
{
private:
    void* do_allocate(size64 Size, size64 Alignment) override;
    void do_deallocate(void* Pointer, size64 Size, size64 Alignment) override;
    bool do_is_equal(const memory_resource& _That) const noexcept override;
};

class RAVEN_API FFrameMemoryResource : public std::pmr::memory_resource
{
public:
    explicit FFrameMemoryResource(size64 InReserverSize, size64 InCommitBlockSize = FVirtualMemoryArena::DefaultCommitBlockSize);
    FFrameMemoryResource(const FFrameMemoryResource&) = delete;
    FFrameMemoryResource(FFrameMemoryResource&&) = delete;
    ~FFrameMemoryResource() override = default;

    FFrameMemoryResource& operator=(const FFrameMemoryResource&) = delete;
    FFrameMemoryResource& operator=(FFrameMemoryResource&&) = delete;

public:
    void Reset(bool bShouldDecommit = false);

    [[nodiscard]] size64 GetUsedSize() const;
    [[nodiscard]] size64 GetCommittedSize() const;
    [[nodiscard]] size64 GetReservedSize() const;

private:
    void* do_allocate(size64 Size, size64 Alignment) override;
    void do_deallocate(void* Pointer, size64 Size, size64 Alignment) override;
    bool do_is_equal(const memory_resource& Other) const noexcept override;

private:
    FVirtualMemoryArena Arena;
};
