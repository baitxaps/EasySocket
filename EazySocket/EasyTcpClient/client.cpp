#include"EasyTcpClient.hpp"
#include"CellTimestamp.hpp"
#include<thread>
#include<atomic>

class MyClient :public EasyTcpClient
{
public:
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
				netmsg_LoginResult* login = (netmsg_LoginResult*)header;
				CellLog::Info("<socket=%d>收到服务端消息：CMD_LOGIN_RESULT,数据长度：%d\n", _pClient->sockfd(), login->dataLength);
			}
			break;

			case CMD_LOGOUT_RESULT:
			{
				netmsg_LogoutResult* logout = (netmsg_LogoutResult*)header;
				CellLog::Info("<socket=%d>收到服务端消息：CMD_LOGOUT_RESULT,数据长度：%d\n", _pClient->sockfd(), logout->dataLength);
			}
			break;

			case CMD_NEW_USER_JOIN:
			{
				netmsg_NewUserJoin* userJoin = (netmsg_NewUserJoin*)header;
				CellLog::Info("<socket=%d>收到服务端消息：CMD_NEW_USER_JOIN,数据长度：%d\n", _pClient->sockfd(), userJoin->dataLength);
			}
			break;

			case CMD_ERROR:
			{
				CellLog::Info("<Socket=%d> recv msgType:CMD_ERROR \n", _pClient->sockfd());
				//CellLog::Info("<socket=%d>收到服务端消息：CMD_ERROR,数据长度：%d\n", _pClient->sockfd(), header->dataLength);
			}
			break;

			default:
			{
				CellLog::Info("error, <socket=%d> recv underine msgType\n",(int)_pClient->sockfd());
			}
		}
	}
private:

};

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
			CellLog::Info("退出cmdThread线程\n");
			break;
		}
		else {
			CellLog::Info("不支持的命令。\n");
		}
	}
}

//客户端数量
const int cCount = 4;//100
//发送线程数量
const int tCount = 4;//4
//客户端数组
EasyTcpClient* client[cCount];
//
std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;

void recvThread(int begin, int end)
{
	CellTimestamp t;
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			if (t.getElapsedSecond()>3.0 && n==begin)
			{
				continue;
			}
			client[n]->OnRun();
		}
	}
}

void sendThread(int id)
{
	CellLog::Info("thread<%d>,start\n", id);
	//4个线程 ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id*c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new MyClient();
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("192.168.0.106", 4567);
	}

	// 心跳检测,死亡计时
	CellLog::Info("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

	readyCount++;
	while (readyCount < tCount)
	{
		CellThread::Sleep(10);
		//std::chrono::milliseconds t(10);
		//std::this_thread::sleep_for(t);
	}

// start recv Thread
	std::thread t1(recvThread, begin,end);
	t1.detach();
//
	netmsg_Login login[1];//10
	for (int n = 0; n < 1; n++)
	{
		strcpy(login[n].userName, "rhc");
		strcpy(login[n].PassWord, "123456");
	}

	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			if (client[n]->SendData(login, nLen) != SOCKET_ERROR)
			{
				sendCount++;
			}
			CellThread::Sleep(10);
			//std::chrono::microseconds t(10);
			//std::this_thread::sleep_for(t);
		}
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
		delete client[n];
	}

	CellLog::Info("thread<%d>,exit\n", id);
}

int main()
{
	CellLog::Instance().setLogPath("clientLog.txt","w");
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread,n+1);
		t1.detach();
	}

	CellTimestamp tTime;
	while (g_bRun)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			printf("thread<%d>,clients<%d>,time<%lf>,sendCount<%d>\n",tCount,cCount,t, (int)(sendCount / t));
			sendCount = 0;
			tTime.update();
		}
		Sleep(1);
	}
	
	CellLog::Info("已退出。\n");
	return 0;
}