#include"EasyTcpClient.hpp"
#include"CELLTimestamp.hpp"
#include<thread>
#include<atomic>

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else {
			printf("不支持的命令。\n");
		}
	}
}

//客户端数量
const int cCount = 100;
//发送线程数量
const int tCount = 4;
//客户端数组
EasyTcpClient* client[cCount];

std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;

void recvThread(int begin, int end)
{
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			client[n]->OnRun();
		}
	}
}

void sendThread(int id)
{
	printf("thread<%d>,start\n", id);
	//4个线程 ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id*c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("192.168.0.106", 4567);
	}

	// 心跳检测,死亡计时
	printf("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

	readyCount++;
	while (readyCount < tCount)
	{
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}

// start recv Thread
	std::thread t1(recvThread, begin,end);
	t1.detach();
//

	Login login[10];
	for (int n = 0; n < 10; n++)
	{
		strcpy(login[n].userName, "rhc");
		strcpy(login[n].PassWord, "123456");
	}

	const int nLen = sizeof(login);
	CELLTimestamp tTime;
	while (g_bRun)
	{
		tTime.update();
		for (int n = begin; n < end; n++)
		{
			if (tTime.getElapsedSecond()>=1.0)
			{
				if (client[n]->SendData(login, nLen) != SOCKET_ERROR)
				{
					sendCount++;
				}
			}
			std::chrono::microseconds t(10);
			std::this_thread::sleep_for(t);
		}
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
		delete client[n];
	}

	printf("thread<%d>,exit\n", id);
}

int main()
{
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread,n+1);
		t1.detach();
	}

	CELLTimestamp tTime;
	while (g_bRun)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			printf("thread<%d>,clients<%d>,time<%lf>,sendCount<%d>\n",tCount,cCount,t,sendCount);
			tTime.update();
			sendCount = 0;
		}

		Sleep(1);
	}
	
	printf("已退出。\n");
	return 0;
}