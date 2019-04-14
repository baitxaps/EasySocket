#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include"Cell.hpp"
#include"CellClient.hpp"
#include"INetEvent.hpp"
#include"CellServer.hpp"

#include<thread>
#include<mutex>
#include<atomic>
#include<memory>

//typedef std::shared_ptr<CellSToCTask> CellSToCTaskPtr;

typedef std::shared_ptr<CellServer> CellServerPtr;
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//��Ϣ��������ڲ��ᴴ���߳�
	//std::vector<CellServer*> _cellServers;
	std::vector<CellServerPtr> _cellServers;
	//ÿ����Ϣ��ʱ
	CELLTimestamp _tTime;
protected:
	//SOCKET recev ��������
	std::atomic_int _recvCount;
	//�ͻ��˼���
	std::atomic_int _clientCount;
	//��Ϣ����
	std::atomic_int _msgCount;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_clientCount = 0;
		_msgCount = 0;
	}

	virtual ~EasyTcpServer()
	{
		Close();
	}

	SOCKET InitSocket()
	{
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ�����...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���socketʧ��...\n");
		}
		else {
			printf("����socket=<%d>�ɹ�...\n", (int)_sock);
		}
		return _sock;
	}

	//��IP�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// bind �����ڽ��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short

#ifdef _WIN32
		if (ip){
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("����,������˿�<%d>ʧ��...\n", port);
		}
		else {
			printf("������˿�<%d>�ɹ�...\n", port);
		}
		return ret;
	}

	//�����˿ں�
	int Listen(int n)
	{
		// 3 listen ��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("socket=<%d>����,��������˿�ʧ��...\n",_sock);
		}
		else {
			printf("socket=<%d>��������˿ڳɹ�...\n", _sock);
		}
		return ret;
	}

	//���ܿͻ�������
	SOCKET Accept()
	{
		// 4 accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
		if (INVALID_SOCKET == cSock)
		{
			printf("socket=<%d>����,���ܵ���Ч�ͻ���SOCKET...\n", (int)_sock);
		}
		else
		{
			//���¿ͻ��˷�����ͻ��������ٵ�cellServer
			addClientToCellServer(std::make_shared<CellClient>(cSock));
			//addClientToCellServer(new CellClient(cSock));
			//��ȡIP��ַ inet_ntoa(clientAddr.sin_addr)
		}
		return cSock;
	}
	
	void addClientToCellServer(CellClientPtr pClient)
	{
		//���ҿͻ��������ٵ�CellServer��Ϣ�������
		auto pMinServer = _cellServers[0];
		for(auto pCellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		OnNetJoin(pClient);
	}

	void Start(int nCellServer)
	{
		for (int n = 0; n < nCellServer; n++)
		{
		//	auto ser = new CellServer(_sock);
			auto ser = std::make_shared<CellServer>(n+1);//_sock
			_cellServers.push_back(ser);
			//ע�������¼����ܶ���
			ser->setEventObj(this);
			//������Ϣ�����߳�
			ser->Start();
		}
	}

	//�ر�Socket
	void Close()
	{
		printf("EasyTcpServer.close start...\n");
		
		if (_sock != INVALID_SOCKET)
		{
	/*		for (auto s:_cellServers)
			{
				delete s;
			}*/
			_cellServers.clear();
#ifdef _WIN32
			//�ر��׽���socket
			closesocket(_sock);
			//���Windows socket����
			WSACleanup();
#else
			close(_sock);
#endif
			printf("EasyTcpServer.close end...\n");
		}
	}
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			time4msg();
			//�������׽��� BSD socket
			fd_set fdRead;//��������socket�� ����
		    // fd_set	fdWrite;
			// fd_set	fdExp;

			//������
			FD_ZERO(&fdRead);
			// FD_ZERO(&fdWrite);
			// FD_ZERO(&fdExp);

			//����������socket�����뼯��
			FD_SET(_sock, &fdRead);
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);

			// nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			// ���������ļ����������ֵ+1 ��Windows�������������д0
			timeval t = { 0,10};
			int ret = select(_sock + 1, &fdRead, 0, 0, &t); //
		    //int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("Accept Select���������\n");
				Close();
				return false;
			}
			//�ж���������socket���Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;
			}
			return true;
		}
		return false;
	}

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//���㲢���ÿ���յ���������Ϣ
	void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			printf("thread<%d>,time<%lf>,socket<%d>,clients<%d>,recvCount<%d>��msgCount<%d>\n",
				_cellServers.size(), t1, _sock,(int)_clientCount, (int)(_recvCount/ t1),(int)(_msgCount/t1));

			_recvCount = 0;
			_msgCount = 0;
			_tTime.update();
		}
	}

	// ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(CellClientPtr& pClient)
	{
		_clientCount++;
	}

	//cellServer  ����̴߳��� ����ȫ	
	virtual void OnNetLeave(CellClientPtr& pClient)
	{
		_clientCount--;
	}

	//cellServer  ����̴߳��� ����ȫ
	virtual void OnNetMsg(CellServer* pCellServe,CellClientPtr& pClient, netmsg_DataHeader* header)
	{
		_msgCount++;
	}

	//recv �¼�
	virtual void OnNetRecv(CellClientPtr& pClient)
	{
		_clientCount++;
	}
};

#endif // !_EasyTcpServer_hpp_
