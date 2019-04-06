#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include"Cell.hpp"

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
};

#endif
