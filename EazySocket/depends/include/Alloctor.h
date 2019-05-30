
#ifndef _ALLOCTOR_H_
#define _ALLOCTOR_H_

void* operator new(size_t size);
void operator delete(void* ptr);
void* operator new[](size_t size);
void operator delete[](void* ptr);
void* mem_alloc(size_t size);
void mem_free(void* prt);

#endif // !_ALLOCTOR_H_
