#pragma once
#include "Allocator.h"

template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(xAlloc(sizeof(Type)));
	// placement new
	new(memory) Type(std::forward<Args>(args)...); // 오른값이건 왼값이건 그대로 넘기는게 forward
	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	xrelease(obj); //BaseAllocator::Release(obj);
}
