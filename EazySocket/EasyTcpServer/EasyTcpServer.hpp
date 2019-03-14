#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#ifdef _WIN32
	#define FD_SETSIZE      1024
	//#pragma comment(lib,"ws2_32.lib")
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
#else
	#include<unistd.h> // uni std
	#include<arpa/inet.h>
	#include<string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR           (-1)
#endif

#define _CELLServer_THREAD_COUNT	4

#include<stdio.h>
#include<vector>
#include"MessageHeader.hpp"
#include"CELLTimestamp.hpp"
#include<thread>
#include<mutex>
#include<atomic>

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

class ClientSocket
{
private:
	SOCKET _sockfd; // fd_set: file desc set
	char _szMsgBuf[RECV_BUFF_SIZE * 10] ;
	int _lastPos ;

public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
		_szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char *msgBuf()
	{
		return _szMsgBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}

	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
};

class CellServer
{
private:
	SOCKET _sock;
	// ��ʽ�ͻ�����
	std::vector<ClientSocket*> _clients;
	// ����ͻ�����
	std::vector<ClientSocket*> _clientsBuff;
	std::mutex _mutex;
	std::thread* _pThread;
public:
	std::atomic_int _recvCount;
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pThread = nullptr;
		_recvCount = 0;
	}

	~CellServer() 
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	bool OnRun()
	{
		while (isRun())
		{
			if (_clientsBuff.size()>0)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient:_clientsBuff) //Fast traversal
				{
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
			}

			if (_clients.empty())  
			{
				// ǰ��û������ʱ���ǳ�ռ�̣߳�����1����
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			// �������׽��� BSD socket 
			fd_set	fdRead;// ������(socket)����
			fd_set	fdWrite;
			fd_set	fdExp;

			// ������
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			// ��������(socket)���뼯����
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			SOCKET maxSock = _clients[0]->sockfd();
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				FD_SET(_clients[i]->sockfd(), &fdRead);
				if (maxSock < _clients[i]->sockfd())
				{
					maxSock = _clients[i]->sockfd();
				}
			}

			// nfds:��һ������ֵ����ָfd_set������������������socket),����������
			// ���������ļ��������ֵ+1,��windows�������������д0
			// t:��0 ��ѯû���������أ�������
			timeval t = { 1,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("select �������...\n");
				Close();
				return false;
			}

			// �ж�������(socket)�Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}

			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				if (FD_ISSET(_clients[i]->sockfd(), &fdRead))
				{
					if (ReceiveData(_clients[i]) == -1)
					{
						auto iter = _clients.begin() + i;//std::vector<SOCKET>::iterator
						if (iter != _clients.end())
						{
							delete _clients[i];
							_clients.erase(iter);
						}
					}
				}
			}
		}
	}

	// ���ܿͻ�������
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (cSock == INVALID_SOCKET)
		{
			printf("<socket=%d>����,������Ч�ͻ���socket...\n", (int)_sock);
		}
		else
		{
			// ÿһ���û�����ʱ��Ⱥ��һ����Ϣ 
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			printf("<socket=%d>�¿ͻ���<%d>����: socket=%d,IP=%s \n", (int)_sock, _clients.size(), (int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

	char szRecv[RECV_BUFF_SIZE] = {};
	// �������� ����ճ��װ ��ְ�
	int ReceiveData(ClientSocket* pClient)
	{
		int nLen = (int)recv(pClient->sockfd(), szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("client<socket=%d> exit,task over.\n", pClient->sockfd());
			return -1;
		}
		// ���յ������ݿ⿽������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(), szRecv, nLen);
		// ��Ϣ������������β������
		pClient->setLastPos(pClient->getLastPos() + nLen);
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader *)pClient->msgBuf();
			if (pClient->getLastPos() >= header->dataLength)
			{
				// ʣ��δ������Ϣ�����������ݳ���
				int nSize = pClient->getLastPos() - header->dataLength;
				OnNetMsg(pClient->sockfd(), header);
				// ��Ϣ�ɳ���ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				// λ��ǰ��
				pClient->setLastPos(nSize);
			}
			else {
				// ��Ϣ����ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	}

	// �Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// �����߳�
	void Start()
	{
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
	 //  std::thread t(std::mem_fun(&CellServer::OnRun), this);
	}

	// ��ǰ��ӵ�еĿͻ�������
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	// �ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				closesocket(_clients[i]->sockfd());
				delete _clients[i];
			}
			closesocket(_sock);
			WSACleanup();
#else
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				close(_clients[i]->sockfd());
				delete _clients[i];
			}
			close(_sock);
#endif
			_clients.clear();
		}
	}
	// ��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET cSock, DataHeader *header)
	{
		_recvCount++;
		//auto t1 = _tTime.getElapsedSecond();
		//if (t1 >= 1.0)
		//{
		//	printf("time<%lf>, socket<%d>,clients<%d>,recvCount<%d> \n", t1, _sock, _clients.size(), _recvCount);
		//	_recvCount = 0;
		//	_tTime.update();
		//}
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login = (Login *)header;
			//printf("�յ��ͻ���<socked=%d> ����:CMD_LOGIN ���ݳ���:%d userName=%s password=%s\n", cSock, login->dataLength, login->userName, login->passWord);
			//  it is correct for judging the login msg 
			// sending head and body data
			//LoginResult ret;
			//SendData(cSock, &ret);
			//	send(cSock, (char*)&ret, sizeof(LogoutResult), 0);
		}
		break;

		case CMD_LOGOUT:
		{
			Logout *logout = (Logout*)header;
			//  printf("�յ��ͻ���<socked=%d> ����CMD_LOGOUT: ���ݳ���:%d userName=%s\n", cSock, logout->dataLength, logout->userName);
			//	LogoutResult result;
			//	SendData(cSock, &result);
		}
		break;

		default:
		{
			printf("�յ������<sock=%d>δ������Ϣ ���ݳ���:%d \n", cSock, header->dataLength);
			//DataHeader ret ;
			//SendData(cSock, &ret);
		}
		break;
		}
	}

	void addClient(ClientSocket *pClient)
	{
		std::lock_guard<std::mutex>lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void SendDataToAll(DataHeader *header)
	{
		for (int i = (int)_clients.size() - 1; i >= 0; i--)
		{
			SendData(_clients[i]->sockfd(), header);
			//send(_clients[i], (const char*)header, header->dataLength, 0);
		}
	}

	// ����ָ��socket����
	int SendData(SOCKET cSock, DataHeader *header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
};

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket *> _clients;
	std::vector<CellServer *> _cellServers;
	CELLTimestamp _tTime;
	
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	// ��ʼ��socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSAData dat;
		WSAStartup(ver, &dat);
#endif
		// ����һ��socket
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ�����...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// 0
		if (INVALID_SOCKET == _sock)
		{
			printf("����, ����<socket=%d>ʧ��...\n", _sock);
		}
		else {
			printf("����<socket=%d>�ɹ�...\n", _sock);
		}
		return _sock;
	}

	// ��IP,�˿ں�
	int Bind(const char *ip,unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}

		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
	
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}	
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = ::bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (ret == SOCKET_ERROR)
		{
			printf("���󣬰�����˿�<%d>ʧ��...\n",port);
		}
		else {
			printf("������˿�<%d>�ɹ�...\n",port);
		}
		return ret;
	}

	// �����˿ں�
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (ret == SOCKET_ERROR) {
			printf("<socket=%d>���󣬼�������˿ں�ʧ��...\n",_sock);
		}
		else {
			printf("<socket=%d>��������˿ںųɹ�...\n",_sock);
		}
		return ret;
	}

	void Start()
	{
		for (int i = 0; i < _CELLServer_THREAD_COUNT; i++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->Start();
		}
	}

	// �ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				closesocket(_clients[i]->sockfd());
				delete _clients[i];
			}
			closesocket(_sock);
			WSACleanup();
#else
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				close(_clients[i]->sockfd());
				delete _clients[i];
			}
			close(_sock);
#endif
			_clients.clear();
		}
	}

	void addClientToCellServer(ClientSocket	*pClient)
	{
		_clients.push_back(pClient);

		auto pMinCellServer = _cellServers[0];
		for (auto PCellServer : _cellServers)
		{
			// ȡ���ٵ�CellServer��Ϣ�������
			if (pMinCellServer->getClientCount()> PCellServer->getClientCount())
			{
				pMinCellServer = PCellServer;
			}
		}
		pMinCellServer->addClient(pClient);
	}

	// ���ܿͻ�������
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		 cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		 cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (cSock == INVALID_SOCKET)
		{
			printf("<socket=%d>����,������Ч�ͻ���socket...\n", (int)_sock);
		}
		else
		{
			// ÿһ���û�����ʱ��Ⱥ��һ����Ϣ 
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			addClientToCellServer(new ClientSocket(cSock));
			//printf("<socket=%d>�¿ͻ���<%d>����: socket=%d,IP=%s \n", (int)_sock, _clients.size(),(int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

	// ����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			timeFormsg();

			// �������׽��� BSD socket 
			fd_set	fdRead;// ������(socket)����
			fd_set	fdWrite;
			fd_set	fdExp;

			// ������
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			// ��������(socket)���뼯����
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			// nfds:��һ������ֵ����ָfd_set������������������socket),����������
			// ���������ļ��������ֵ+1,��windows�������������д0
			// t:��0 ��ѯû���������أ�������
			timeval t = { 0,10 };
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("select �������...\n");
				Close();
				return false;
			}

			// �ж�������(socket)�Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;
			}
		}
	return false;
	}

	// �Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// ��Ӧ������Ϣ
    void timeFormsg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			int recvCount = 0;
			for (auto ser : _cellServers)
			{
				recvCount += ser->_recvCount;
				ser->_recvCount = 0;
			}
				
			printf("time<%lf>, socket<%d>,clients<%d>,recvCount<%d> \n",t1, _sock,_clients.size(), recvCount);
			_tTime.update();
		}

	}
};

#endif