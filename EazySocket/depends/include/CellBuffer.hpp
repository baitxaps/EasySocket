#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_
#include"Cell.hpp"

class CellBuffer
{
public:
	CellBuffer(int nSize = 8192)// default size:8k
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}

	~CellBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}	
	}

	char* data()
	{
		return _pBuff;
	}

	bool push(const char* pData,int nLen)
	{
		// д��������ݿⲻһ��Ҫ�ŵ��ڴ��У�Ҳ���Դ洢�����ݿ���ߴ��̴洢����
		// ��Ҫд������ݴ��ڿ��ÿռ�
		if (_nLast + nLen > _nSize)
		{
			// ��չbuff,8KB
			int n = _nLast = nLen - _nSize;
			if (n < 8192) n = 8192;
			char *buff = new char[_nSize + n];
			memcpy(buff, _pBuff, _nLast);
			delete[] _pBuff;
			_pBuff = buff;
		}

		if (_nLast + nLen <= _nSize)
		{
			//�����͵����� ���������ͻ�����β��
			memcpy(_pBuff + _nLast, pData, nLen);
			// ����β��λ��
			_nLast += nLen;
			if (_nLast == SEND_BUFF_SIZE)
			{
				_buffFullCount++;
			}
			return true;
		}
		else {
			_buffFullCount++;
		}
		return false;
	}
	
	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + _nLast, n);
		}
		_nLast = n;

		if (_buffFullCount > 0)
		{
			_buffFullCount--;
		}
	}

	// the cache data  send to client just now
	// business requirement to change
	int wireteTosocket(SOCKET sockfd)
	{
		int ret = 0;
		// cache data >0
		if (_nLast > 0 && INVALID_SOCKET != sockfd)
		{
			ret = send(sockfd, _pBuff, _nLast, 0);
			_nLast = 0;
			_buffFullCount = 0;
		}
		return ret;
	}

	int readForSocket(SOCKET sockfd)
	{
		if (_nSize - _nLast > 0)
		{
			// ������
			char *szRecv = _pBuff + _nLast;
			//  ���տͻ�������
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);

			if (nLen <= 0)
			{
				//CellLog::Info("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
				return nLen;
			}
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	bool hasMsg()
	{
		// �ж���Ϣ�����������ݳ��ȴ�����Ϣͷnetmsg_DataHeader����
		if (_nLast >= sizeof(netmsg_DataHeader))
		{
			// ��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
			// �ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			bool fc = _nLast >= header->dataLength;
			return fc;
		}
		return false;
	}

private:
	// �ڶ������� ���ͻ�����
	char* _pBuff = nullptr; 
	//list<char*> _puBuffer;
	// ���ͻ�����������β��λ��
	int _nLast;
	// �������ܵĿռ��С���ֽڳ���
	int _nSize = 0;
	// ������д�������ļ���
	int _buffFullCount = 0;
};


#endif