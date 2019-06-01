#include<iostream>
#include"Alloctor.h"
#include"CELLTimestamp.hpp"
#include<thread>
#include<mutex>
#include<memory>
#include"CELLObjectPool.hpp"

using namespace std;
mutex m;
const int tCount = 8;
const int mCount = 100000;
const int nCount = mCount / tCount;
void workFun(int index)
{
	char* data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = new char[(rand() % 128) + 1];
	}
	for (size_t i = 0; i < nCount; i++)
	{
		delete[] data[i];
	}
}//ÇÀÕ¼Ê½

class TestOBjectPool:public ObjectPoolBase<TestOBjectPool,1000>
{
public:
	TestOBjectPool()
	{
		printf("TestObjectPool constructor\n");
	}

	TestOBjectPool(int n) 
	{
		this->n = n; 
		printf("TestObjectPool constructor %d\n",n);
	}

	TestOBjectPool(int n,float b) 
	{
		this->n = n;
		this->b = b;
		printf("TestObjectPool constructor n= %d,b= %f\n", n,b);
	}

	~TestOBjectPool ()
	{ 
		printf("TestObjectPool destrouctor\n"); 
	}

private:
	int n;
	float b;
};


int main()
{
	thread t[tCount];
	for (int n = 0; n < tCount; n++)
	{
		t[n] = thread(workFun, n);
	}
	CellTimestamp tTime;
	for (int n = 0; n < tCount; n++)
	{
		t[n].join();
		//t[n].detach();
	}
	cout << tTime.getElapsedTimeInMilliSec() << endl;

	//TestOBjectPool *n1 = new TestOBjectPool();
	//delete n1;

	//TestOBjectPool *n2 = TestOBjectPool::createObject(5);
	//TestOBjectPool::destoryObject(n2);

	//TestOBjectPool *n3 = new TestOBjectPool(5,6);
	//TestOBjectPool::destoryObject(n3);

	//shared_ptr<int>b = make_shared<int>();
	cout << "Hello,main thread." << endl;
	system("pause");

	return 0;
}


