#ifndef PSRAM_ALLOCATOR_H
#define PSRAM_ALLOCATOR_H

#include <Arduino.h>

extern "C"
{
	void *pmalloc(size_t size);
}

template <class T>
struct PsramAllocator
{
	using value_type = T;

	PsramAllocator() noexcept {}
	template <class U>
	PsramAllocator(const PsramAllocator<U> &) noexcept {}

	T *allocate(std::size_t n)
	{
		if (auto p = static_cast<T *>(pmalloc(n * sizeof(T))))
		{
			return p;
		}
		throw std::bad_alloc();
	}

	void deallocate(T *p, std::size_t /*n*/) noexcept
	{
		if (p)
		{
			free(p);
		}
	}
};

#endif // PSRAM_ALLOCATOR_H