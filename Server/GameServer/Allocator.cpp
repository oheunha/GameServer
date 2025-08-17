#include "pch.h"
#include "Allocator.h"
/*
	BaseAllocator
*/
void* BaseAllocator::Alloc(int32_t size)
{
    return ::malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}
