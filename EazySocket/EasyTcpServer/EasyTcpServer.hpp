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
	// 正式客户队列
	std::vector<ClientSocket*> _clients;
	// 缓冲客户队列
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
				// 前期没有数据时，非常占线程，休眠1毫秒
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			// 伯克利套接字 BSD socket 
			fd_set	fdRead;// 描述符(socket)集合
			fd_set	fdWrite;
			fd_set	fdExp;

			// 清理集合
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			// 将描述符(socket)加入集合中
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

			// nfds:是一个整数值，是指fd_set集合中所有描述符（socket),而不是数量
			// 即是所有文件描述最大值+1,在windows中这个参数可以写0
			// t:是0 查询没有立即返回，不阻塞
			timeval t = { 1,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("select 任务结束...\n");
				Close();
				return false;
			}

			// 判断描述符(socket)是否在集合中
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

	// 接受客户端连接
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
			printf("<socket=%d>错误,接受无效客户端socket...\n", (int)_sock);
		}
		else
		{
			// 每一个用户加入时，群发一条消息 
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			printf("<socket=%d>新客户端<%d>加入: socket=%d,IP=%s \n", (int)_sock, _clients.size(), (int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

	char szRecv[RECV_BUFF_SIZE] = {};
	// 接收数据 处理粘包装 拆分包
	int ReceiveData(ClientSocket* pClient)
	{
		int nLen = (int)recv(pClient->sockfd(), szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("client<socket=%d> exit,task over.\n", pClient->sockfd());
			return -1;
		}
		// 将收到的数据库拷贝到消息缓冲区
		memcpy(pClient->msgBuf() + pClient->getLastPos(), szRecv, nLen);
		// 消息级冲区的数据尾部后移
		pClient->setLastPos(pClient->getLastPos() + nLen);
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader *)pClient->msgBuf();
			if (pClient->getLastPos() >= header->dataLength)
			{
				// 剩余未处理消息缓冲区的数据长度
				int nSize = pClient->getLastPos() - header->dataLength;
				OnNetMsg(pClient->sockfd(), header);
				// 消息缴冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				// 位置前移
				pClient->setLastPos(nSize);
			}
			else {
				// 消息缓冲剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
	}

	// 是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// 启动线程
	void Start()
	{
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
	 //  std::thread t(std::mem_fun(&CellServer::OnRun), this);
	}

	// 当前所拥有的客户端数量
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	// 关闭socket
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
	// 响应网络消息
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
			//printf("收到客户端<socked=%d> 请求:CMD_LOGIN 数据长度:%d userName=%s password=%s\n", cSock, login->dataLength, login->userName, login->passWord);
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
			//  printf("收到客户端<socked=%d> 请求CMD_LOGOUT: 数据长度:%d userName=%s\n", cSock, logout->dataLength, logout->userName);
			//	LogoutResult result;
			//	SendData(cSock, &result);
		}
		break;

		default:
		{
			printf("收到服务端<sock=%d>未定义消息 数据长度:%d \n", cSock, header->dataLength);
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

	// 发送指定socket数据
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

	// 初始化socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSAData dat;
		WSAStartup(ver, &dat);
#endif
		// 建立一个socket
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧连接...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// 0
		if (INVALID_SOCKET == _sock)
		{
			printf("错误, 建立<socket=%d>失败...\n", _sock);
		}
		else {
			printf("建立<socket=%d>成功...\n", _sock);
		}
		return _sock;
	}

	// 绑定IP,端口号
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
			printf("错误，绑定网络端口<%d>失败...\n",port);
		}
		else {
			printf("绑定网络端口<%d>成功...\n",port);
		}
		return ret;
	}

	// 监听端口号
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (ret == SOCKET_ERROR) {
			printf("<socket=%d>错误，监听网络端口号失败...\n",_sock);
		}
		else {
			printf("<socket=%d>监听网络端口号成功...\n",_sock);
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

	// 关闭socket
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
			// 取最少的CellServer消息处理对像
			if (pMinCellServer->getClientCount()> PCellServer->getClientCount())
			{
				pMinCellServer = PCellServer;
			}
		}
		pMinCellServer->addClient(pClient);
	}

	// 接受客户端连接
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
			printf("<socket=%d>错误,接受无效客户端socket...\n", (int)_sock);
		}
		else
		{
			// 每一个用户加入时，群发一条消息 
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			addClientToCellServer(new ClientSocket(cSock));
			//printf("<socket=%d>新客户端<%d>加入: socket=%d,IP=%s \n", (int)_sock, _clients.size(),(int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

	// 处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			timeFormsg();

			// 伯克利套接字 BSD socket 
			fd_set	fdRead;// 描述符(socket)集合
			fd_set	fdWrite;
			fd_set	fdExp;

			// 清理集合
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			// 将描述符(socket)加入集合中
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			// nfds:是一个整数值，是指fd_set集合中所有描述符（socket),而不是数量
			// 即是所有文件描述最大值+1,在windows中这个参数可以写0
			// t:是0 查询没有立即返回，不阻塞
			timeval t = { 0,10 };
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("select 任务结束...\n");
				Close();
				return false;
			}

			// 判断描述符(socket)是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;
			}
		}
	return false;
	}

	// 是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// 响应网络消息
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