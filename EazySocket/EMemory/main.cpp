#include<iostream>
#include"Alloctor.h"
#include"CELLTimestamp.hpp"
#include<thread>
#include<mutex>

#define kArrayCount 1100
#define kBlockSize  1024

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

	char* dat[kArrayCount] ;
	for (int i = 0; i < kArrayCount; i++)
	{
		dat[i] = new char[1 +i];
		//dat[i] = new char[1+(rand()%kBlockSize)];
	}

	for (int i = 0; i < kArrayCount; i++)
	{
		delete[] dat[i];
	}

	std::cout << "hello,main "<< std::endl;
	system("pause");

	return 0;
}


