#include"EasyTcpServer.hpp"
#include<thread>
#if 0
#define kAllServers
#endif

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
			printf("退出cmdThread线程.\n");
			break;
		}
		else{
			printf("不支持的命令.\n");
		}
	}
}

int main()
{

#ifdef kAllServers
	EasyTcpServer server1;
	server1.InitSocket();
	server1.Bind(nullptr, 4567);
	server1.Listen(5);

	EasyTcpServer server2;
	server2.InitSocket();
	server2.Bind(nullptr, 4568);
	server2.Listen(5);

	while (server1.isRun() || server2.isRun())
	{
		server1.OnRun();
		server2.OnRun();
	}
	server1.Close();
	server2.Close();
#else
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);

	// thread start(UI thread)
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{
		server.OnRun();
	}
	server.Close();
#endif
	printf("已退出.\n");
	getchar();

	return 0;
}