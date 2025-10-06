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

/*
	STL Allocator
*/
template<typename T>
class StlAllocator
{
public:
	using value_type = T;

	StlAllocator() {}

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) {}
	
	T* allocate(size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(xAlloc(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		xrelease(ptr); 
	}

};