#pragma once
/*
	BaseAllocator
*/
class BaseAllocator
{
public:
	static void * Alloc(int32_t size);
	static void Release(void* ptr);
};

