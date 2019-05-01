#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include"Cell.hpp"
#include"CellBuffer.hpp"

#include"CellLog.hpp"

// 60 seconds client heart dead time 
#define CELIENT_HREAT_DEAD_TIME 60000
// 200ms send cache data to client
#define CLIENT_SEND_BUFF_TIME 200

typedef std::shared_ptr<netmsg_DataHeader> DataHeaderPtr;
typedef std::shared_ptr<netmsg_LoginResult> LoginResultPtr;

//客户端数据类型
class CellClient :public ObjectPoolBase<CellClient, 10000>
{
	// for test
public:
	int id = -1;
	int serverId = -1;
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET):_sendBuff(SEND_BUFF_SIZE),_recvBuf(RECV_BUFF_SZIE)
	{
		static int n = 1;
		id = n++;

		_sockfd = sockfd;
		resetDTheart();
		resetDTSend();
	}

	~CellClient()
	{
		CellLog::Info("s=%d,CellClient%d.~CellClient...\n",serverId, id);
		if (_sockfd != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	int RecvData()
	{
		return _recvBuf.readForSocket(_sockfd);
	}

	bool hasMsg()
	{
		return _recvBuf.hasMsg();
	}

	netmsg_DataHeader* front_msg()
	{
		return (netmsg_DataHeader*)_recvBuf.data();
	}

	void pop_front_msg()
	{
		if(hasMsg())
		 _recvBuf.pop(front_msg()->dataLength);
	}

	// 是否有数据需要写
	bool needWrite()
	{
		return _sendBuff.needWrite();
	}

	// 立即将发送缓冲区的数据发送给客户端
	int SendDataReal()
	{
		resetDTSend();
		int ret = _sendBuff.wireteTosocket(_sockfd);
		return ret;
	}

	//发送数据, 定时定量
	int SendData(netmsg_DataHeader* header)
	{
		// 要发送的长度
		int nSendLen = header->dataLength;
		// 要发送的数据
		const char *pSendData = (const char*)header;

		if (_sendBuff.push(pSendData, nSendLen))
		{
			return header->dataLength;
		}
		return SOCKET_ERROR;
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
			CellLog::Info("checkHeart dead:s=%d,time = %d", _sockfd, (int)_dtHeart);
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
			CellLog::Info("checkSend:s=%d,time = %d", _sockfd, (int)_dtSend);
			SendDataReal();
			resetDTSend();
		}
		return false;
	}

private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	// 第二缓冲区 接收消息缓冲区
	CellBuffer _recvBuf;
	// 发送缓冲区
	CellBuffer _sendBuff;
	// the heart time of socket 
	time_t _dtHeart;
	// send data timeStamp 
	time_t _dtSend;
	// cachebuff data was full
	int _sendBuffFullCount=0;
};

#endif
