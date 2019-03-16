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
	// ��ʼ��socket
	void InitSocket()
	{
#ifdef _WIN32
		// ����Win Sock 2.x����
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
	}

	// ���ӷ�����
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
			printf("����<socket=%d>, ���ӷ����<%s:%d>ʧ��...\n", _sock, ip, port);
		}
		else {
			printf("<socket=%d>���ӷ����<%s:%d>�ɹ�...\n", _sock, ip, port);
		}
		return ret;
	}

	// 	�ر�WinSock 2.x����
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

	// ��ѯ������Ϣ
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
				printf("<socket=%d>select �������1.\n", _sock);
				Close();
				return false;
			}

			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == ReceiveData(_sock))
				{
					printf("<socket=%d>select �������2 \n", _sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// �Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//�ڶ�����������Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 5] = {};
	// ���ջ�����
	char _szRecv[RECV_BUFF_SIZE] = {};
	int _lastPos = 0;

	// ��������,����ճ������ְ�
	int ReceiveData(SOCKET _cSock)
	{
		int nLen = recv(_cSock, (char*)&_szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("<socket=%d>��������Ͽ����� �������.\n", _cSock);
			return -1;
		}

		// ���յ������ݿ⿽������Ϣ������
		memcpy(_szMsgBuf + _lastPos , _szRecv, nLen);
		// ��Ϣ������������β������
		_lastPos += nLen;
		while (_lastPos >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader *)_szMsgBuf;
			if (_lastPos >= header->dataLength) 
			{
				// ʣ��δ������Ϣ�����������ݳ���
				int nSize = _lastPos - header->dataLength;
				OnNetMsg(header);
				// ��Ϣ�ɳ���ʣ��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				// λ��ǰ��
				_lastPos = nSize;
			}
			else {
				// ��Ϣ����ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	}

	// ��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *loginresult = (LoginResult *)header;
			 printf("�յ�����<sock=%d>����Ϣ:CMD_LOGIN_RESULT ���ݳ���:%d \n", _sock, loginresult->dataLength);
		}
		break;

		case CMD_LOGOUT_RESULT:
		{
			LogoutResult *logoutresult = (LogoutResult *)header;
		//  printf("�յ������<sock=%d>��Ϣ:CMD_LOGOUT_RESULT ���ݳ���:%d \n", _sock, logoutresult->dataLength);
		}
		break;

		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin *join = (NewUserJoin *)header;
			//printf("�յ������<sock=%d>��Ϣ:CMD_NEW_USER_JOIN ���ݳ���:%d \n", _sock, join->dataLength);
		}
		break;

		case CMD_ERROR:
		{
			printf("�յ������<sock=%d>CMD_ERROR ��Ϣ ���ݳ���:%d \n", _sock, header->dataLength);
		}
		break;

		default:
		{
			printf("�յ������<sock=%d>δ������Ϣ ���ݳ���:%d \n", _sock, header->dataLength);
		}
		}
	}

	// ��������
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