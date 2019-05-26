#include "EasyTcpServer.hpp"
#include<thread>
#include"Alloctor.h"
#include"CellLog.hpp"

#if 0
#define kAllServers
#endif

// !<<<NOTE>>>
// formater IDE 
// ctrl+shift+f:find;ctrl+k,ctrl+f:Format alignment  

// complier tools
// g++ -std=c++11 -pthread server.cpp -o server

// common libiary
// properties -> c/c++ ->general -> additoinal include directories

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

class MyServer : public EasyTcpServer
{
public:
	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(CellClient* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	 // CellLog::Info("client<%d> join\n", pClient->sockfd());
	}
	//cellServer ����̴߳��� ����ȫ
	virtual void OnNetLeave(CellClient* pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	 // CellLog::Info("client<%d> leave\n", pClient->sockfd());
	}

	//cellServer ����̴߳��� ����ȫ
	virtual void OnNetMsg(CellServer* pCellServer, CellClient *pClient, netmsg_DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer,pClient,header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTheart();
			netmsg_Login* login = (netmsg_Login*)header;
			CellLog::Info("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n",pClient->sockfd(), login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			netmsg_LoginResult ret;
			if (SOCKET_ERROR == pClient->SendData(&ret))
			{
				CellLog::Info("<Socket=%d> Send Full \n",pClient->sockfd());
			// cache is full
			}

			//netmsg_LoginResult *ret = new netmsg_LoginResult();
			//auto ret = std::make_shared<netmsg_LoginResult>();
			//pCellServer->addSendTask(pClient,(DataHeaderPtr)ret);
		}
		break;
		case CMD_LOGOUT:
		{
			netmsg_Logout* logout = (netmsg_Logout*)header;
			CellLog::Info("�յ��ͻ���<Socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s \n", pClient->sockfd(),logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			netmsg_LogoutResult ret;
			//pClient->SendData(&ret);
		}
		break;

		case CMD_C2S_HEART:
		{
			pClient->resetDTheart();
		//	netmsg_c2s_Heart ret;
		//	pClient->SendData(&ret);

		//	auto ret = std::make_shared<netmsg_c2s_Heart>();
		//	pCellServer->addSendTask(pClient, (DataHeaderPtr)ret);

			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
		}
		break;

		default:
		{
			CellLog::Info("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", pClient->sockfd(), header->dataLength);
			netmsg_DataHeader ret;
		//	pClient->SendData(&ret);
		}
		break;
		}
	}
};

int main()
{
	CellLog::Instance().setLogPath("serverLog.txt","w");
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
	//std::thread t1(cmdThread);
	//t1.detach();

	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			CellLog::Info("�˳�cmdThread�߳�\n");
			server.Close();
			break;
		}
		else {
			CellLog::Info("��֧�ֵ����\n");
		}
	}
	
#endif
	CellLog::Info("���˳���\n");
	getchar();
	return 0;
}