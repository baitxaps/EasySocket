#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#ifdef _WIN32
	#define FD_SETSIZE      2506
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
#include<map>

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

class INetEvent
{
public:
	//客户端加入事件
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	// 客户端离开事件
	virtual void OnLeave(ClientSocket* pClient) = 0;
	//客户端消息事件
	virtual void OnNetMsg(ClientSocket *pClient, DataHeader* header) = 0;
private:

};

class CellServer
{
private:
	SOCKET _sock;
	// 正式客户队列
	std::map<SOCKET, ClientSocket*> _clients;
	// 缓冲客户队列
	std::vector<ClientSocket*> _clientsBuff;
	std::mutex _mutex;
	std::thread* _pThread;
	INetEvent* _pNetEvent;
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pThread = nullptr;
		_pNetEvent = nullptr;
	}

	~CellServer() 
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//备份客户socket fd_set
	fd_set _fdRead_bak;
	//客户列表是否有变化
	bool _clients_change;
	// 只用来读取客户端发送的数据
	SOCKET _maxSock;
	bool OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			// 从缓冲队列里取出客户数据
			if (_clientsBuff.size()>0)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient:_clientsBuff) //Fast traversal
				{
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
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

			//只做读处理 清理集合
			FD_ZERO(&fdRead);

			if (_clients_change)
			{
				_clients_change = false;
				// 将描述符(socket)加入集合中
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter:_clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock <iter.second->sockfd())
					{
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else {
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}

			// nfds:是一个整数值，是指fd_set集合中所有描述符（socket),而不是数量
			// 即是所有文件描述最大值+1,在windows中这个参数可以写0
			// t:是0 查询没有立即返回，不阻塞 timeval t = { 0,0 };
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr,nullptr);
			if (ret < 0)
			{
				printf("select 任务结束...\n");
				Close();
				return false;
			}
			else if (ret == 0) 
			{
				continue;
			}

#ifdef _WIN32
			for (int i = 0; i < fdRead.fd_count; i++)
			{
				auto iter = _clients.find(fdRead.fd_array[i]);
				if (iter != _clients.end())
				{
					if (ReceiveData(iter->second))
					{
						if (_pNetEvent)
							_pNetEvent->OnLeave(_clients[i]);
						_clients_change = true;
						_clients.erase(iter->first);
					}
				}
				else {
					printf("error. if (iter != _clients.end())\n");
				}
			}
#else
			std::vector<ClientSocket*> temp;
			for (auto iter:_clients)
			{
				if (FD_ISSET(iter.second->sockfd(), &fdRead))
				{
					if (-1 == RecvData(iter.second))
					{
						if (_pNetEvent)
							_pNetEvent->OnNetLeave(iter.second);
						_clients_change = false;
						temp.push_back(iter.second);
					}
				}
			}

			for (auto pClient : temp)
			{
				_clients.erase(pClient->sockfd());
				delete pClient;
			}
#endif // _WIN32
		}
	}

	char szRecv[RECV_BUFF_SIZE] = {};
	// 接收数据 处理粘包装 拆分包
	int ReceiveData(ClientSocket* pClient)
	{
		int nLen = (int)recv(pClient->sockfd(), szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("client<socket=%d> 退出,任务结束.\n", pClient->sockfd());
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
				OnNetMsg(pClient, header);
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
			for (auto iter:_clients)
			{
				closesocket(iter.second->sockfd());
				delete iter.second;
			}
			closesocket(_sock);
			WSACleanup();
#else
			for (auto iter:_clients)
			{
				close(iter.second->sockfd());
				delete iter.second;
			}
			close(_sock);
#endif
			_clients.clear();
		}
	}

	// 响应网络消息
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader *header)
	{
		_pNetEvent->OnNetMsg(pClient,header);
	}

	void addClient(ClientSocket *pClient)
	{
		std::lock_guard<std::mutex>lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}
};


class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	std::vector<ClientSocket *> _clients;
	std::vector<CellServer *> _cellServers;
	CELLTimestamp _tTime;
protected:
	//收到消息计数
	std::atomic_int _recvCount;
	//客户端计数
	std::atomic_int _clientCount;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_clientCount = 0;
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
			ser->setEventObj(this);
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
			if (pMinCellServer->getClientCount() > PCellServer->getClientCount())
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
		//	fd_set	fdWrite;
		//	fd_set	fdExp;

			// 清理集合
			FD_ZERO(&fdRead);
		//	FD_ZERO(&fdWrite);
		//	FD_ZERO(&fdExp);

			// 将描述符(socket)加入集合中
			FD_SET(_sock, &fdRead);
		//	FD_SET(_sock, &fdWrite);
		//	FD_SET(_sock, &fdExp);

			// nfds:是一个整数值，是指fd_set集合中所有描述符（socket),而不是数量
			// 即是所有文件描述最大值+1,在windows中这个参数可以写0
			// t:是0 查询没有立即返回，不阻塞
			timeval t = { 0,10 };
			int ret = select(_sock + 1, &fdRead, nullptr, nullptr, &t);
		//	int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
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
			printf("thread<%d>,time<%lf>, socket<%d>,clients<%d>,recvCount<%d> \n",_cellServers.size(),t1, _sock,_clients.size(), (int)(_recvCount / t1));
			_recvCount = 0;
			_tTime.update();
			_tTime.update();
		}
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

	// 客户端离开事件 cellServer 4 多个线程触发 不安全
	virtual void OnLeave(ClientSocket* pClient)
	{
		_clientCount--;
	}

	//只会被一个线程触发 安全
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
	}

	// cellServer 4 多个线程触发 不安全
	virtual void OnNetMsg(ClientSocket *pClient, DataHeader* header)
	{
		_recvCount++;
	}
};


#endif