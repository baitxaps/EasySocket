#include"EasyTcpClient.hpp"
#include<thread>

bool g_bRun = true;
// UI 线程
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程.\n");
			break;
		}
		else
		{
			printf("不支持的命令.\n");
		}
	}
}


const int cCount = 1000;
const int tCount = 4;
EasyTcpClient* client[cCount];
// 发送线程
void sendThread(int id)
{
	// 4个线程 id:1-4
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id * c;

	for (int i = begin; i < end; i++)
	{
		client[i] = new EasyTcpClient();
	}

	for (int i = begin; i < end; i++)
	{
		client[i]->InitSocket();
		client[i]->Connect("192.168.0.106", 4568);// oip 111.67.206.137	vip:192.168.239.128 lip:192.168.0.106
	}

	std::chrono::microseconds t(5000);
	std::this_thread::sleep_for(t);

	Login login[10];
	for (int i = 0; i < 10; i++)
	{
		strcpy(login[i].userName, "rhc");
		strcpy(login[i].passWord, "123456");
	}
	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->SendData(login,nLen);// 发送数
		//	client[i]->OnRun();// 接收数据
		}
		// 空闲状态，处理其他事务
		// Sleep(1000);
	}

	for (int i = begin; i < end; i++ )
	{
		client[i]->Close();
		delete client[i];
	}
	printf("thread<%d>,exit\n", id);
}

int main()
{
	// UI thread
	std::thread t1(cmdThread);
	t1.detach();

	//cmd  thread 
	for (int i = 0; i < tCount; i++)
	{
		std::thread t1(sendThread,i+1);
		t1.detach();
	}

	while (g_bRun) Sleep(100);

	printf("已退出.任务结束.\n");
	return 0;
}