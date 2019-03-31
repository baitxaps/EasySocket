#include "EasyTcpServer.hpp"
#include<thread>
#include"Alloctor.h"

#if 0
#define kAllServers
#endif

bool g_bRun = true;
void cmdThread() // ctrl+shift+f:find
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else {
			printf("��֧�ֵ����\n");
		}
	}
}

class MyServer : public EasyTcpServer
{
private:

public:
	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(ClientSocketPtr& pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	 // printf("client<%d> join\n", pClient->sockfd());
	}
	//cellServer ����̴߳��� ����ȫ
	virtual void OnNetLeave(ClientSocketPtr& pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	 // printf("client<%d> leave\n", pClient->sockfd());
	}

	//cellServer ����̴߳��� ����ȫ
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer,pClient,header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			Login* login = (Login*)header;
			printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n",pClient->sockfd(), login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//LoginResult ret;
			//pClient->SendData(&ret);

			//LoginResult *ret = new LoginResult();
			auto ret = std::make_shared<LoginResult>();
			pCellServer->addSendTask(pClient,(DataHeaderPtr)ret);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s \n", pClient->sockfd(),logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LogoutResult ret;
			//pClient->SendData(&ret);
		}
		break;
		default:
		{
			printf("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", pClient->sockfd(), header->dataLength);
			DataHeader ret;
		//	pClient->SendData(&ret);
		}
		break;
		}
	}
};

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

	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(4);

	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{
		server.OnRun();
		//printf("����ʱ�䴦������ҵ��..\n");
	}
	server.Close();
#endif
	printf("���˳���\n");
	getchar();
	return 0;
}