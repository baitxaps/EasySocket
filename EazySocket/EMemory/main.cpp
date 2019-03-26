#include<iostream>
#include"Alloctor.h"
#include"CELLTimestamp.hpp"
#include<thread>
#include<mutex>

#define kArrayCount 1100
#define kBlockSize  1024

using namespace std;

std::mutex m;
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

int main()
{
	thread t[tCount];
	for (int n = 0; n < tCount; n++)
	{
		t[n] = thread(workFun, n);
	}
	CELLTimestamp tTime;
	for (int n = 0; n < tCount; n++)
	{
		t[n].join();
		//t[n].detach();
	}
	cout << tTime.getElapsedTimeInMilliSec() << endl;
	cout << "Hello,main thread." << endl;
	system("pause");

	return 0;
}


