#include "EasyTcpServer.hpp"
#include"CellMsgStream.hpp"

#if 0
#define kAllServers
#endif

// !<<<NOTE>>>
// formater IDE 
// ctrl+shift+f:find;ctrl+k,ctrl+f:Format alignment  ctrl+M :layer 

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
		   //CellLog::Info("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n",pClient->sockfd(), login->dataLength, login->userName, login->PassWord);
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
			CellReadStream r(header);
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
			///
			CellWriteStream s(128);
			s.setNetCmd(CMD_LOGOUT_RESULT);
			s.WriteInt8(n1);
			s.WriteInt16(n2);
			s.WriteInt32(n3);
			s.WriteFloat(n4);
			s.WriteDouble(n5);
			s.WriteArray(name, strlen(name));
			s.WriteArray(pw, strlen(pw));
			s.WriteArray(ata, n8);
			s.finsh();
			pClient->SendData(s.data(), s.length());
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
			server.Close();
			break;
		}
		else {
			CellLog::Info("undefine cmd\n");
		}
	}
	
#endif
	CellLog::Info("exit.\n");
	getchar();
	return 0;
}