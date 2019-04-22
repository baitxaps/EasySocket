#ifndef _MemoryMgr_HPP_
#define _MemoryMgr_HPP_

#include<stdlib.h>
#include<assert.h>
#include<mutex>

#ifdef _DEBUG
	#ifndef xPrintf
		#include<stdio.h>
		#define xPrintf(...)// printf(__VA_ARGS__)
	#endif
#else
	#ifndef xPrintf
		#define xPrintf(...)
	#endif
#endif // _DEBUG

#define MAX_MEMORY_SIZE 1024
class MemoryAlloc;

//the min memory block 
class MemoryBlock
{
public:
	// memory block ID
	int nID;
	// the reference of count
	int nRef;
	// the Max of memory Block(pool)
	MemoryAlloc* pAlloc;
	// the next of postion
	MemoryBlock* pNext;
	// id in pool 
	bool bPool;

private:
	// to use on the future
	char cl;
	char c2;
	char c3;
};

//const int memoryBlock = sizeof(MemoryBlock);

// memory pool
class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockSize = 0;
	}

	virtual ~MemoryAlloc()
	{
		if (_pBuf) free(_pBuf);
	}

	void* allocMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (!_pBuf)
		{
			initMemory();
		}

		MemoryBlock *pReturn = nullptr;
		// out of range 
		if (nullptr == _pHeader)
		{
			pReturn = (MemoryBlock *)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocMem: %x, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return (char*)pReturn + sizeof(MemoryBlock);
	}

	void freeMemory(void* ptr)
	{
		MemoryBlock *pBlock = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);

		// 在池中指到头部，在池外直接释放
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0) return;

			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			if (--pBlock->nRef != 0) return;
			free(pBlock);
		}
	}

	// Initialize
	void initMemory()
	{
		assert(nullptr ==_pBuf);

		if (_pBuf) return;

		// contructor memory pool
		size_t realSzie = _nSize + sizeof(MemoryBlock);
		size_t bufSize = realSzie*_nBlockSize;

		_pBuf = (char*)malloc(bufSize);
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		
		// Initialize
		MemoryBlock* pftmp = _pHeader;
		for (size_t n = 1; n < _nBlockSize; n++)
		{
			// address  offset
			MemoryBlock* curTmp =(MemoryBlock*)(_pBuf + (n * realSzie));
			curTmp->bPool = true;
			curTmp->nID = n;
			curTmp->nRef = 0;
			curTmp->pAlloc = this;
			curTmp->pNext = nullptr;
			pftmp->pNext = curTmp;
			pftmp = curTmp;
		}
	}

protected:
	// memorypool address
	char* _pBuf;
	// header of memory unit
	MemoryBlock* _pHeader;
	// memory unit size
	size_t _nSize;
	// memory unit count
	size_t _nBlockSize;
	// lock
	std::mutex _mutex;
};


template<size_t nSize, size_t bBlockSize> //template<typename nSize, typename bBlockSize>
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		// 内存对齐
		const size_t n = sizeof(void *);
		_nSize = nSize/n *n + (nSize%n ? n:0);

		_nBlockSize = bBlockSize;
	}

	virtual~MemoryAlloctor() {}
private:

};


// memory manager tools
class MemoryMgr
{
public:
	// single instance 
	static MemoryMgr& Instance()
	{
		static MemoryMgr mgr;
		return mgr;
	}

	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SIZE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else {
			MemoryBlock *pReturn = (MemoryBlock *)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			
			// print the pointer address %llx ,cpu 64bit																																																																									 
			xPrintf("allocMem:%x,id=%d,size=%d\n",pReturn,pReturn->nID,nSize);
			return  (char*)pReturn + sizeof(MemoryBlock);
		}
	}

	// free the memory
	void freeMem(void* ptr)
	{
		MemoryBlock *pBlock = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
		xPrintf("freeMem:%x,id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)
		{
			pBlock->pAlloc->freeMemory(ptr);
		}
		else {
			if (--pBlock->nRef == 0) 
				free(pBlock);
		}
	}

	// increase the memory block ref count .
	void addRef(void* pMem)
	{
		MemoryBlock *pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++ pBlock->nRef;
	}

private:
	MemoryMgr()
	{
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		init_szAlloc(129, 256, &_mem256);
		init_szAlloc(257, 512, &_mem512);
		init_szAlloc(513, 1024, &_mem1024);
	}

	~MemoryMgr() {}

	// initial  memorypool array  to  one-to-one mapping    
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* mPmemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = mPmemA;
		}
	}

	MemoryAlloctor<64, 100> _mem64;
	MemoryAlloctor<128, 100> _mem128;
	MemoryAlloctor<256, 100> _mem256;
	MemoryAlloctor<512, 100> _mem512;
	MemoryAlloctor<1024,1000> _mem1024;
//	MemoryAlloctor<1024, 1000000> _mem1024;//1G
	// to find the index of _szAlloc 
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
};



#endif // !_MemoryMgr_HPP_
