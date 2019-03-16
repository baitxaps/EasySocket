#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
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

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

#include<stdio.h>
#include<thread>
#include"MessageHeader.hpp"

class EasyTcpClient
{
private:
	SOCKET _sock;
//	char _szRecv[RECV_BUFF_SIZE];

public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
		_szRecv[RECV_BUFF_SIZE] = {};
	}

	virtual ~EasyTcpClient()
	{
		Close();
	}
	// 初始化socket
	void InitSocket()
	{
#ifdef _WIN32
		// 启动Win Sock 2.x环境
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
	}

	// 连接服务器
	int Connect(const char *ip, short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(4567);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (ret == SOCKET_ERROR)
		{
			printf("错误<socket=%d>, 连接服务端<%s:%d>失败...\n", _sock, ip, port);
		}
		else {
			printf("<socket=%d>连接服务端<%s:%d>成功...\n", _sock, ip, port);
		}
		return ret;
	}

	// 	关闭WinSock 2.x环境
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
	}

	// 查询网络消息
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = {0,0};
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			if (ret < 0)
			{
				printf("<socket=%d>select 任务结束1.\n", _sock);
				Close();
				return false;
			}

			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == ReceiveData(_sock))
				{
					printf("<socket=%d>select 任务结束2 \n", _sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// 是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//第二缓冲区，消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 5] = {};
	// 接收缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	int _lastPos = 0;

	// 接收数据,处理粘包，拆分包
	int ReceiveData(SOCKET _cSock)
	{
		int nLen = recv(_cSock, (char*)&_szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("<socket=%d>与服务器断开连接 任务结束.\n", _cSock);
			return -1;
		}

		// 将收到的数据库拷贝到消息缓冲区
		memcpy(_szMsgBuf + _lastPos , _szRecv, nLen);
		// 消息级冲区的数据尾部后移
		_lastPos += nLen;
		while (_lastPos >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader *)_szMsgBuf;
			if (_lastPos >= header->dataLength) 
			{
				// 剩余未处理消息缓冲区的数据长度
				int nSize = _lastPos - header->dataLength;
				OnNetMsg(header);
				// 消息缴冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				// 位置前移
				_lastPos = nSize;
			}
			else {
				// 消息缓冲剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
	}

	// 响应网络消息
	virtual void OnNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *loginresult = (LoginResult *)header;
			 printf("收到服务<sock=%d>端消息:CMD_LOGIN_RESULT 数据长度:%d \n", _sock, loginresult->dataLength);
		}
		break;

		case CMD_LOGOUT_RESULT:
		{
			LogoutResult *logoutresult = (LogoutResult *)header;
		//  printf("收到服务端<sock=%d>消息:CMD_LOGOUT_RESULT 数据长度:%d \n", _sock, logoutresult->dataLength);
		}
		break;

		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin *join = (NewUserJoin *)header;
			//printf("收到服务端<sock=%d>消息:CMD_NEW_USER_JOIN 数据长度:%d \n", _sock, join->dataLength);
		}
		break;

		case CMD_ERROR:
		{
			printf("收到服务端<sock=%d>CMD_ERROR 消息 数据长度:%d \n", _sock, header->dataLength);
		}
		break;

		default:
		{
			printf("收到服务端<sock=%d>未定义消息 数据长度:%d \n", _sock, header->dataLength);
		}
		}
	}

	// 发送数据
	int SendData(DataHeader *header,int nLen)
	{
		int ret = SOCKET_ERROR;
		if (isRun() && header)
		{
			ret= send(_sock, (const char*)header, nLen, 0);// send(_sock, (const char*)header, header->dataLength, 0);
			if (ret == SOCKET_ERROR)
			{
				Close();
			}
		}
		return ret;
	}
};

#endif