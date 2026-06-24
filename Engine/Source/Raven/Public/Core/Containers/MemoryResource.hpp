// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <memory_resource>

class FMemoryResource : public std::pmr::memory_resource
{
private:
	void* do_allocate(size_t Size, size_t Alignment) override;
	void do_deallocate(void* Pointer, size_t Size, size_t Alignment) override;
	bool do_is_equal(const memory_resource& OtherMemoryResource) const noexcept override;

public:
	static FMemoryResource* Get();
};
