
#include"Alloctor.h"
#include"MemoryMgr.hpp"

void* operator new(size_t nSize)
{
	return	MemoryMgr::Instance().allocMem(nSize);
}

void operator delete(void* ptr)
{
	MemoryMgr::Instance().freeMem(ptr);
}

void* operator new[](size_t size)
{
	return	MemoryMgr::Instance().allocMem(size);
}

void operator delete[](void* ptr)
{
	MemoryMgr::Instance().freeMem(ptr);
}

void* mem_alloc(size_t size)
{
	return	malloc(size);
}

void mem_free(void* ptr)
{
	free(ptr);
}
