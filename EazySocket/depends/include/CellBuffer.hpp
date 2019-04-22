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
		// 写入大量数据库不一定要放到内存中，也可以存储到数据库或者磁盘存储器中
		// 需要写入的数据大于可用空间
		if (_nLast + nLen > _nSize)
		{
			// 扩展buff,8KB
			int n = _nLast = nLen - _nSize;
			if (n < 8192) n = 8192;
			char *buff = new char[_nSize + n];
			memcpy(buff, _pBuff, _nLast);
			delete[] _pBuff;
			_pBuff = buff;
		}

		if (_nLast + nLen <= _nSize)
		{
			//将发送的数据 拷贝到发送缓冲区尾部
			memcpy(_pBuff + _nLast, pData, nLen);
			// 数据尾部位置
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
			// 缓冲区
			char *szRecv = _pBuff + _nLast;
			//  接收客户端数据
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);

			if (nLen <= 0)
			{
				//CellLog::Info("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
				return nLen;
			}
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	bool hasMsg()
	{
		// 判断消息缓冲区的数据长度大于消息头netmsg_DataHeader长度
		if (_nLast >= sizeof(netmsg_DataHeader))
		{
			// 这时就可以知道当前消息的长度
			netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
			// 判断消息缓冲区的数据长度大于消息长度
			bool fc = _nLast >= header->dataLength;
			return fc;
		}
		return false;
	}

private:
	// 第二缓冲区 发送缓冲区
	char* _pBuff = nullptr; 
	//list<char*> _puBuffer;
	// 发送缓冲区的数据尾部位置
	int _nLast;
	// 缓冲区总的空间大小，字节长度
	int _nSize = 0;
	// 缓冲区写满字数的计数
	int _buffFullCount = 0;
};


#endif