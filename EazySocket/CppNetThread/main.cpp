#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<thread>
#include<mutex>
#include<atomic>
#include<stdio.h>
#include"LockDataThread.hpp"
#include"NetworkManager.hpp"

using namespace std;
mutex m;
const int tCount = 4;
// 原子锁
atomic<int> sum = 0;
void workRunction(int index)
{
	for (int i = 0; i < 400000; i++)
	{
		// 自解锁
	//	lock_guard<mutex>lg(m);
	//  m.lock();
		sum++;
	//	cout << "index=" << index<<"hello,other thread"<<"i="<<i<< endl;
	//	m.unlock();
	}
}

// 静态变量初始化
NetworkManager* NetworkManager::m_instance = NULL;

void threadSingleIntanceTest()
{
	std::cout << "start ..." << std::endl;
	NetworkManager* m = NetworkManager::GetInstance();
	std::cout << "end..." << std::endl;
}

int main()
{
	//LockDataThread obj;
	//std::thread outMsgObj(&LockDataThread::outMsgRecvQueue, &obj);
	//std::thread intMsgObj(&LockDataThread::lock_defer_inMsgRecvQueue, &obj);
	//intMsgObj.join();
	//outMsgObj.join();

	// 主线程中单例创建好，然后在子线程中使用即可。
	//single instance;
	//NetworkManager* m = NetworkManager::GetInstance();
	//m->test();

	std::thread obj1(threadSingleIntanceTest);
	std::thread obj2(threadSingleIntanceTest);
	obj1.join();
	obj2.join();


	cout << "hello,main thread sum="<<sum << endl;
	system("pause");
//	while (true) {}
	return 0;
}

void threadCmd()
{
	thread t[tCount];
	for (int i = 0; i < tCount; i++)
	{
		t[i] = thread(workRunction, i);
	}

	for (int i = 0; i < tCount; i++)
	{
		t[i].detach();
	//	t[i].join();
	}
}
void Test()
{
	int p = 0x123456;
	printf("*(&p)=%x\n", *(&p));//0x123456
//	printf("*((int*)&p)=%x\n", *((int*)0x002df89c));

	// 32bit cup
	int arr[10];
	// 相减是间隔元素个数
	printf("&arr[9]-&arr[4]=%d\n", &arr[9] - &arr[4]);//5
	// 纯粹是数值类型数据相减
	printf("(int)arr[9]-(int)arr[4]=%d\n", (int)&arr[9] - (int)&arr[4]);//20
}
void func0()
{
	printf("func0()");
}
void func1()
{
	printf("func1()");
}
void func2()
{
	printf("func2()");
}
void test()
{
	void(*pfArr[3])() = { &func0,&func1,&func2 };
	int choice;
	printf("pls input your choice\n");
	std::scanf("%d", &choice);
	while (choice >= 0 && choice < 3)
	{
		//(*pfArr[choice])();
		pfArr[choice]();
		scanf("%d", &choice);
	}
	printf(" your put err select\n");
}
