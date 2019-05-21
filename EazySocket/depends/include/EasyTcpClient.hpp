#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#include"Cell.hpp"
#include"CellNetWork.hpp"
#include"MessageHeader.hpp"
#include"CellClient.hpp"

//��������С��Ԫ��С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 1024 
#endif 

class EasyTcpClient
{
protected:
	CellClient* _pClient = nullptr;
	bool _isConnect;

public:
	EasyTcpClient()
	{
		_isConnect = false;
	}
	
	virtual ~EasyTcpClient()
	{
		Close();
	}

	//��ʼ��socket
	void InitSocket()
	{
		CellNetWork::Init();
		if (_pClient)
		{
			CellLog::Info("warning,initSocket close old socket<%d>...\n", (int)_pClient->sockfd());
			Close();
		}
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sock)
		{
			CellLog::Info("error��create socket failure...\n");
		}
		else {
			CellLog::Info("create socket=<%d>success...\n", (int)sock);
			_pClient = new CellClient(sock);
		}
	}

	//���ӷ�����
	int Connect(const char* ip,unsigned short port)
	{
		if (!_pClient)
		{
			InitSocket();
		}
		// 2 ���ӷ����� connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		CellLog::Info("<socket=%d> connecting <%s:%d>...\n", (int)_pClient->sockfd(), ip, port);
		int ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			CellLog::Info("<socket=%d> connect <%s:%d>failed...\n", (int)_pClient->sockfd(), ip, port);
		}
		else {
			_isConnect = true;
			CellLog::Info("<socket=%d> connect <%s:%d>success...\n", (int)_pClient->sockfd(), ip, port);
		}
		return ret;
	}

	//�ر��׽���closesocket
	void Close()
	{
		if (_pClient)
		{
			delete 	_pClient;
			_pClient = nullptr;
		}
		_isConnect = false;
	}

	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			SOCKET _sock = _pClient->sockfd();
			fd_set fdRead;
			fd_set fdWrite;
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_SET(_sock, &fdRead);
		
			int ret = 0;
			timeval t = { 0,0 };
			if (_pClient->needWrite())
			{
				FD_SET(_sock, &fdWrite);
				ret = select(_sock + 1, &fdRead, &fdWrite, nullptr, &t);
			}
			else {
				ret = select(_sock + 1, &fdRead, nullptr, nullptr, &t);
			}

			if (ret < 0)
			{
				CellLog::Info("error,<socket=%d>OnRun.select exit\n", (int)_sock);
				Close();
				return false;
			}

			if (FD_ISSET(_sock, &fdRead))
			{
				if (-1 == RecvData(_sock))
				{
					CellLog::Info("error,<socket=%d>OnRun.select RecvData exit\n", (int)_sock);
					Close();
					return false;
				}
			}

			if (FD_ISSET(_sock, &fdWrite))
			{
				if (-1 ==_pClient->SendDataReal())
				{
					CellLog::Info("error,<socket=%d>OnRun.select SendDataReal exit\n", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	//�Ƿ�����
	bool isRun()
	{
		return _pClient && _isConnect;
	}

	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SZIE] = {};
	//��Ϣ������������β��λ��
	int _lastPos = 0;

	//���ջ�����char _szRecv[RECV_BUFF_SZIE] = {};

	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET cSock)
	{
		if (isRun())
		{
			//  ���տͻ�������
			int nLen = _pClient->RecvData();
			if (nLen >= 0)
			{
				// ѭ�� �ж��Ƿ�����Ϣ�账��
				while (_pClient->hasMsg())
				{
					//����������Ϣ
					OnNetMsg(_pClient->front_msg());
					// �Ƴ���Ϣ���У�����������ǰ��һ������
					_pClient->pop_front_msg();
				}
			}
			return nLen;
		}
		return 0;
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(netmsg_DataHeader* header) = 0;

	int SendData(netmsg_DataHeader* header)
	{
		if (isRun())
		{
			return _pClient->SendData(header);
		}
		return 0;
	}
	
	//��������
	int SendData(const char* pData,int nLen)
	{
		if (isRun())
		{
			int ret = _pClient->SendData(pData,nLen);
			return ret;
		}
		return 0;
	}
};

#endif