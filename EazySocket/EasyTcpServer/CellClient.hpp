#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include"Cell.hpp"
// 60 seconds client heart dead time 
#define CELIENT_HREAT_DEAD_TIME 60000
// 200ms send cache data to client
#define CLIENT_SEND_BUFF_TIME 200

typedef std::shared_ptr<netmsg_DataHeader> DataHeaderPtr;
typedef std::shared_ptr<netmsg_LoginResult> LoginResultPtr;

//�ͻ�����������
class CellClient :public ObjectPoolBase<CellClient, 10000>
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SZIE);
		_lastPos = 0;
		//memset(_szMsgBuf, 0, sizeof(_szMsgBuf));

		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
		_lastSendPos = 0;

		resetDTheart();
		resetDTSend();
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* msgBuf()
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


	int SendDataReal(DataHeaderPtr& header)
	{
		SendData(header);
		SendDataReal();
	}

	// the cache data  send to client just now
	int SendDataReal()
	{
		int ret = SOCKET_ERROR;
		// cache data >0
		if (_lastSendPos > 0 && SOCKET_ERROR != _sockfd)
		{
			ret = send(_sockfd, _szSendBuf, _lastSendPos, 0);
			_lastSendPos = 0;
			resetDTSend();
		}
		return ret;
	}

	//��������, ��ʱ����
	int SendData(DataHeaderPtr& header)
	{
		int ret = SOCKET_ERROR;
		// Ҫ���͵ĳ���
		int nSendLen = header->dataLength;
		// Ҫ���͵�����
		const char *pSendData = (const char*)header.get();

		while (true)
		{
			if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE)
			{
				// �ɿ��������ݳ���
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
				// ��������
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				// ָ��ƫ�ƣ���������λ��
				pSendData += nCopyLen;
				// ����ʣ�����ݳ���
				nSendLen -= nCopyLen;

				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);

				// ret = send(_sockfd, (const char*)header, header->dataLength, 0);
				
				_lastSendPos = 0;

				resetDTSend();
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else {
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				// ����β��λ��
				_lastSendPos += nSendLen;
				break;
			}
		}
		return ret;
	}

	void resetDTheart()
	{
		_dtHeart = 0;
	}

	void resetDTSend()
	{
		_dtSend = 0;
	}

	// check the socket heart
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CELIENT_HREAT_DEAD_TIME)
		{
			printf("checkHeart dead:s=%d,time = %d", _sockfd, (int)_dtHeart);
			return true;
		}
		return false;
	}

	// check the Send
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			printf("checkSend:s=%d,time = %d", _sockfd, (int)_dtSend);
			SendDataReal();
			resetDTSend();
		}
		return false;
	}

private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SZIE];
	//��Ϣ������������β��λ��
	int _lastPos;

	//�ڶ������� ���ͻ�����
	char _szSendBuf[SEND_BUFF_SIZE];
	//���ͻ�����������β��λ��
	int _lastSendPos;
	// the heart time of socket 
	time_t _dtHeart;
	//  send data timeStamp 
	time_t _dtSend;
};

#endif
