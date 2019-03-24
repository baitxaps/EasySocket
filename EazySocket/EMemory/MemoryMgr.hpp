#ifndef _MemoryMgr_HPP_
#define _MemoryMgr_HPP_

#include<stdlib.h>
#include<assert.h>

#define MAX_MEMORY_SIZE 64
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

		return (char*)pReturn + sizeof(MemoryBlock);
	}

	void freeMemory(void* ptr)
	{
		// char* pDat = (char*)ptr;
		MemoryBlock *pBlock = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));

		assert(1 == pBlock->nRef);

		if (--pBlock->nRef != 0) return;

		// 在池中指到头部，在池外直接释放
		if (pBlock->bPool)
		{
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			free(pBlock);
		}
	}

	// Initialize
	void initMemory()
	{
		assert(nullptr ==_pBuf);

		if (_pBuf) return;

		// contructor memory pool
		size_t buffSize = (_nSize + sizeof(MemoryBlock)) *_nBlockSize;
		_pBuf = (char*)malloc(buffSize);
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
			MemoryBlock* curTmp =(MemoryBlock*)(_pBuf + (n * _nSize));
			curTmp = (MemoryBlock*)_pBuf;
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
			return  ((char*)(pReturn + sizeof(MemoryBlock)));
		}
	}

	// free the memory
	void freeMem(void* ptr)
	{
		MemoryBlock *pBlock = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
		int a = sizeof(MemoryBlock);
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
		init(0, 64, &_mem64);
	}

	~MemoryMgr() {}

	// initial  memorypool array  to  one-to-one mapping    
	void init(int nBegin, int nEnd, MemoryAlloc* mPmemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = mPmemA;
		}
	}

	MemoryAlloctor<64, 10> _mem64;
	MemoryAlloctor<128, 10> _mem128;

	// to find the index of _szAlloc 
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
};



#endif // !_MemoryMgr_HPP_
