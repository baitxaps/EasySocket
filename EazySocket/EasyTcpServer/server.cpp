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
			CellLog::Info("退出cmdThread线程\n");
			break;
		}
		else {
			CellLog::Info("不支持的命令。\n");
		}
	}
}

class MyServer : public EasyTcpServer
{
public:
	//只会被一个线程触发 安全
	virtual void OnNetJoin(CellClient* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	 // CellLog::Info("client<%d> join\n", pClient->sockfd());
	}
	//cellServer 多个线程触发 不安全
	virtual void OnNetLeave(CellClient* pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	 // CellLog::Info("client<%d> leave\n", pClient->sockfd());
	}

	//cellServer 多个线程触发 不安全
	virtual void OnNetMsg(CellServer* pCellServer, CellClient *pClient, netmsg_DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer,pClient,header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTheart();
			netmsg_Login* login = (netmsg_Login*)header;
			CellLog::Info("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n",pClient->sockfd(), login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
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
			CellLog::Info("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", pClient->sockfd(),logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
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
			CellLog::Info("<socket=%d>收到未定义消息,数据长度：%d\n", pClient->sockfd(), header->dataLength);
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

	//启动UI线程
	//std::thread t1(cmdThread);
	//t1.detach();

	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			CellLog::Info("退出cmdThread线程\n");
			server.Close();
			break;
		}
		else {
			CellLog::Info("不支持的命令。\n");
		}
	}
	
#endif
	CellLog::Info("已退出。\n");
	getchar();
	return 0;
}