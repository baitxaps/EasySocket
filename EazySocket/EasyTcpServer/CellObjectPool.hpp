#ifndef _CellObjectPOOL_HPP_
#define _CellObjectPOOL_HPP_

#include<assert.h>
#include<mutex>

#ifdef _DEBUG
	#ifndef xPrintf
		#include<stdio.h>
		#define xPrintf(...) printf(__VA_ARGS__)
	#endif
#else
	#ifndef xPrintf
		#define xPrintf(...)
	#endif
#endif // _DEBUG

template<class T, size_t nPoolSize> 
class CELLObjectPool
{
private:
	struct NodeHeader
	{
	public:
		// memory block ID
		int nID;
		// the reference of count
		char nRef;
		// the next of postion
		NodeHeader* pNext;
		// id in pool 
		bool bPool;
	private:
		// to use on the future
		char cl;
		char c2;
	};

	// initialize  object pool
	void initPool()
	{
		assert(nullptr == _pBuf);
		if (_pBuf)return;
		
		size_t realSize = sizeof(T) + sizeof(NodeHeader);
		size_t n = nPoolSize * realSize;
		_pBuf = new char[n];

		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;

		// Initialize
		NodeHeader* pftmp = _pHeader;
		for (size_t n = 1; n < nPoolSize; n++)
		{
			// address  offset
			NodeHeader* curTmp = (NodeHeader*)(_pBuf + (n * realSize));
			curTmp->bPool = true;
			curTmp->nID = n;
			curTmp->nRef = 0;
			curTmp->pNext = nullptr;
			pftmp->pNext = curTmp;
			pftmp = curTmp;
		}
	}

public:
	CELLObjectPool()
	{
		initPool();
	}

	~CELLObjectPool()
	{
		if(_pBuf)
		 delete[] _pBuf;
	}

	// release the object
	void freeObjMemory(void* ptr)
	{
		NodeHeader *pBlock = (NodeHeader*)((char*)ptr - sizeof(NodeHeader));
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
			delete[] pBlock;
		}
	}

	// get the Object
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (!_pBuf)
		{
			initPool();
		}

		NodeHeader *pReturn = nullptr;
		// out of range 
		if (nullptr == _pHeader)
		{
			pReturn = (NodeHeader *)new char[sizeof(T) + sizeof(NodeHeader)];
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xCellLog::Info("allocMem: %x, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return (char*)pReturn + sizeof(NodeHeader);
	}

private:
	// header of object unit
	NodeHeader* _pHeader;
	// objectpool address
	char* _pBuf;
	// lock
	std::mutex _mutex;
//	const int n = sizeof(NodeHeader);
};


template<class T, size_t nPoolSize>
class ObjectPoolBase
{
public:
	ObjectPoolBase(){}
	~ObjectPoolBase(){}

	void* operator new(size_t nSize)
	{
		return objectPool().allocObjMemory(nSize);
	}

	void operator delete(void* p)
	{
		objectPool().freeObjMemory(p);
	}
	
	// mutable paramaters for  function
	template<typename ...Args>
	static T* createObject(Args ... args)
	{
		T* obj = new T(args...);
		return obj;
	}

	static void destoryObject(T* obj)
	{
		delete obj;
	}
private:
	typedef CELLObjectPool<T, nPoolSize> ClassTPool;
	// single instance 
	static ClassTPool& objectPool()
	{
		static ClassTPool sPool;
		return sPool;
	}
};


#endif