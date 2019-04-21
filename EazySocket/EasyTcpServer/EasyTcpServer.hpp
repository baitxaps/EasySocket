#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include"Cell.hpp"
#include"CellClient.hpp"
#include"INetEvent.hpp"
#include"CellServer.hpp"
#include"CellNetWork.hpp"

#include<thread>
#include<mutex>
#include<atomic>
#include<memory>

typedef std::shared_ptr<CellServer> CellServerPtr;
class EasyTcpServer : public INetEvent
{
private:
	CellThread _thread;
	//消息处理对象，内部会创建线程
	//std::vector<CellServer*> _cellServers;
	std::vector<CellServerPtr> _cellServers;
	//每秒消息计时
	CELLTimestamp _tTime;
	// Socket
	SOCKET _sock;
protected:
	//SOCKET recev 函数计数
	std::atomic_int _recvCount;
	//客户端计数
	std::atomic_int _clientCount;
	//消息数量
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
		CellNetWork::Init();

		if (INVALID_SOCKET != _sock)
		{
			CellLog::Info("warning,initSocket close old socket<%d>...\n", (int)_sock);
			Close();
		}

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CellLog::Info("error，create socket failure...\n");
		}
		else {
			CellLog::Info("create socket=<%d>success...\n", (int)_sock);
		}
		return _sock;
	}

	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// bind 绑定用于接受客户端连接的网络端口
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
			CellLog::Info("错误,绑定网络端口<%d>失败...\n", port);
		}
		else {
			CellLog::Info("绑定网络端口<%d>成功...\n", port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n)
	{
		// 3 listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			CellLog::Info("socket=<%d>错误,监听网络端口失败...\n",_sock);
		}
		else {
			CellLog::Info("socket=<%d>监听网络端口成功...\n", _sock);
		}
		return ret;
	}

	//接受客户端连接
	SOCKET Accept()
	{
		// 4 accept 等待接受客户端连接
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
			CellLog::Info("socket=<%d>错误,接受到无效客户端SOCKET...\n", (int)_sock);
		}
		else
		{
			//将新客户端分配给客户数量最少的cellServer
			addClientToCellServer(std::make_shared<CellClient>(cSock));
			//addClientToCellServer(new CellClient(cSock));
			//获取IP地址 inet_ntoa(clientAddr.sin_addr)
		}
		return cSock;
	}
	
	void addClientToCellServer(CellClientPtr pClient)
	{
		//查找客户数量最少的CellServer消息处理对象
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
			//注册网络事件接受对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}

		_thread.Start(
			nullptr,
			[this](CellThread* pThead) {
			OnRun(pThead);
		});
	}

	//关闭Socket
	void Close()
	{
		CellLog::Info("EasyTcpServer.close start...\n");
		_thread.Close();
		if (_sock != INVALID_SOCKET)
		{
	/*		for (auto s:_cellServers)
			{
				delete s;
			}*/
			_cellServers.clear();
#ifdef _WIN32
			//关闭套节字socket
			closesocket(_sock);
#else
			close(_sock);
#endif
			CellLog::Info("EasyTcpServer.close end...\n");
		}
	}

	// 只会被一个线程触发 安全
	virtual void OnNetJoin(CellClientPtr& pClient)
	{
		_clientCount++;
	}

	//cellServer  多个线程触发 不安全	
	virtual void OnNetLeave(CellClientPtr& pClient)
	{
		_clientCount--;
	}

	//cellServer  多个线程触发 不安全
	virtual void OnNetMsg(CellServer* pCellServe,CellClientPtr& pClient, netmsg_DataHeader* header)
	{
		_msgCount++;
	}

	//recv 事件
	virtual void OnNetRecv(CellClientPtr& pClient)
	{
		_clientCount++;
	}

private:
	//处理网络消息
	void OnRun(CellThread* pThread)
	{
		while (pThread->isRun())
		{
			time4msg();
			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket） 集合
						  // fd_set	fdWrite;
						  // fd_set	fdExp;

						  //清理集合
			FD_ZERO(&fdRead);
			// FD_ZERO(&fdWrite);
			// FD_ZERO(&fdExp);

			//将描述符（socket）加入集合
			FD_SET(_sock, &fdRead);
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);

			// nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			// 既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t = { 0,1 };
			int ret = select(_sock + 1, &fdRead, 0, 0, &t); //
															//int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				CellLog::Info("EasyTcpServer.OnRun select exit.\n");
				pThread->Exit();
				break;
			}
			//判断描述符（socket）是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
		}
	}

	//计算并输出每秒收到的网络消息
	void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			CellLog::Info("thread<%d>,time<%lf>,socket<%d>,clients<%d>,recvCount<%d>，msgCount<%d>\n",
				_cellServers.size(), t1, _sock, (int)_clientCount, (int)(_recvCount / t1), (int)(_msgCount / t1));

			_recvCount = 0;
			_msgCount = 0;
			_tTime.update();
		}
	}
};

#endif 
