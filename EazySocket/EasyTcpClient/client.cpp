#include"EasyTcpClient.hpp"
#include"CellTimestamp.hpp"
#include<thread>
#include<atomic>
#include"CELLMsgStream.hpp"

class MyClient : public EasyTcpClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			netmsg_LoginResult* login = (netmsg_LoginResult*)header;
			//CellLog::Info("<socket=%d> recv msgType：CMD_LOGIN_RESULT\n", (int)_pClient->sockfd());
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			netmsg_LogoutResult* logout = (netmsg_LogoutResult*)header;
			//CellLog::Info("<socket=%d> recv msgType：CMD_LOGOUT_RESULT\n", (int)_pClient->sockfd());
			CellReadStream r(header);
			//读取消息长度
			r.ReadInt16();
			//读取消息命令
			r.getNetCmd();
			auto n1 = r.ReadInt8();
			auto n2 = r.ReadInt16();
			auto n3 = r.ReadInt32();
			auto n4 = r.ReadFloat();
			auto n5 = r.ReadDouble();
			uint32_t n = 0;
			r.onlyRead(n);
			char name[32] = {};
			auto n6 = r.ReadArray(name, 32);
			char pw[32] = {};
			auto n7 = r.ReadArray(pw, 32);
			int ata[10] = {};
			auto n8 = r.ReadArray(ata, 10);
			CellLog::Info("<socket=%d> recv msgType：CMD_LOGOUT_RESULT\n", (int)_pClient->sockfd());

		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			netmsg_NewUserJoin* userJoin = (netmsg_NewUserJoin*)header;
			//CellLog::Info("<socket=%d> recv msgType：CMD_NEW_USER_JOIN\n", (int)_pClient->sockfd());
		}
		break;
		case CMD_ERROR:
		{
			CellLog::Info("<socket=%d> recv msgType：CMD_ERROR\n", (int)_pClient->sockfd());
		}
		break;
		default:
		{
			CellLog::Info("error, <socket=%d> recv undefine msgType\n", (int)_pClient->sockfd());
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
const int cCount = 1000;//1000
//发送线程数量
const int tCount = 4;
//客户端数组
EasyTcpClient* client[cCount];
std::atomic_int sendCount(0);
std::atomic_int readyCount(0);

void recvThread(int begin, int end)
{
	CellTimestamp t;
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			//if (t.getElapsedSecond() > 3.0 && n == begin)
			//	continue;
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
		client[n]->Connect("192.168.0.107", 4567);
	}
	//心跳检测 死亡计时 
	CellLog::Info("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

	readyCount++;
	while (readyCount < tCount)
	{//等待其它线程准备好发送数据
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}
	//
	std::thread t1(recvThread, begin, end);
	t1.detach();
	//
	netmsg_Login login[1];
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
			if (SOCKET_ERROR != client[n]->SendData(login))
			{
				sendCount++;
			}
		}
		std::chrono::milliseconds t(99);
		std::this_thread::sleep_for(t);
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
	CellLog::Instance().setLogPath("clientLog.txt", "w");
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread, n + 1);
		t1.detach();
	}

	CellTimestamp tTime;

	while (g_bRun)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			CellLog::Info("thread<%d>,clients<%d>,time<%lf>,send<%d>\n", tCount, cCount, t, (int)(sendCount / t));
			sendCount = 0;
			tTime.update();
		}
	std::chrono::milliseconds ts(1);
	std::this_thread::sleep_for(ts);
	}

	CellLog::Info("已退出。\n");
	return 0;
}