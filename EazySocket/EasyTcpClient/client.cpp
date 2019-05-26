#include"EasyTcpClient.hpp"
#include"CellTimestamp.hpp"
#include<thread>
#include<atomic>

class MyClient : public EasyTcpClient
{
public:
	//��Ӧ������Ϣ
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			netmsg_LoginResult* login = (netmsg_LoginResult*)header;
			//CellLog::Info("<socket=%d> recv msgType��CMD_LOGIN_RESULT\n", (int)_pClient->sockfd());
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			netmsg_LogoutResult* logout = (netmsg_LogoutResult*)header;
			//CellLog::Info("<socket=%d> recv msgType��CMD_LOGOUT_RESULT\n", (int)_pClient->sockfd());
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			netmsg_NewUserJoin* userJoin = (netmsg_NewUserJoin*)header;
			//CellLog::Info("<socket=%d> recv msgType��CMD_NEW_USER_JOIN\n", (int)_pClient->sockfd());
		}
		break;
		case CMD_ERROR:
		{
			CellLog::Info("<socket=%d> recv msgType��CMD_ERROR\n", (int)_pClient->sockfd());
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
			CellLog::Info("�˳�cmdThread�߳�\n");
			break;
		}
		else {
			CellLog::Info("��֧�ֵ����\n");
		}
	}
}

//�ͻ�������
const int cCount = 10;//1000
//�����߳�����
const int tCount = 4;
//�ͻ�������
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
			if (t.getElapsedSecond() > 3.0 && n == begin)
				continue;
			client[n]->OnRun();
		}
	}
}

void sendThread(int id)
{
	CellLog::Info("thread<%d>,start\n", id);
	//4���߳� ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id*c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new MyClient();
	}
	for (int n = begin; n < end; n++)
	{
		//win10 "192.168.1.102" i5 6300
		//win7 "192.168.1.114" i7 2670qm
		//127.0.0.1
		//39.108.13.69
		//ubuntu vm 192.168.74.141
		//macOS vm 192.168.74.134
		client[n]->Connect("192.168.0.107", 4567);
	}
	//������� ������ʱ 
	CellLog::Info("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

	readyCount++;
	while (readyCount < tCount)
	{//�ȴ������߳�׼���÷�������
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
	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	//���������߳�
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

	CellLog::Info("���˳���\n");
	return 0;
}