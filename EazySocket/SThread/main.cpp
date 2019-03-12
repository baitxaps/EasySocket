#include<iostream>
#include<thread>
#include<mutex>
#include<atomic>

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

int main()
{
	thread t[tCount];
	for (int i = 0; i < tCount; i++)
	{
		t[i] = thread(workRunction,i);
	}

	for (int i = 0; i < tCount; i++)
	{
		t[i].detach();
	//	t[i].join();
	}

	cout << "hello,main thread sum="<<sum << endl;
	system("pause");
	while (true) {}
	return 0;
}