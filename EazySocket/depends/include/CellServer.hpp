#ifndef _CellServer_Hpp_
#define _CellServer_Hpp_

#include"Cell.hpp"
#include"INetEvent.hpp"
#include"CellClient.hpp"
#include"CellSemaphore.hpp"
#include<vector>
#include<map>

// network message task class  to send 

class CellSToCTask :public CellTask
{
private:
	CellClient _pClient;
	netmsg_DataHeader	_pHeader;

public:
	CellSToCTask(CellClient& pClient, netmsg_DataHeader& pHeader)
	{
		_pClient = pClient;
		_pHeader = pHeader;
	}

	virtual void doTask()
	{
		//_pClient->SendData(_pHeader);
		//delete _pHeader;
	}
};

// network msg for handling class
class CellServer
{
private:
	//正式客户队列
	std::map<SOCKET, CellClient*> _clients;
	//缓冲客户队列
	std::vector<CellClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	//网络事件对象
	INetEvent* _pNetEvent;
	//
	CellTaskServer _taskServer;
	//备份客户socket fd_set
	fd_set _fdRead_bak;
	SOCKET _maxSock;
	// old  timestamp
	time_t _oldTime = CellTime::getNowInMilliSec();
	//
	CellThread _thread;
	//
	int _id = -1;
	//客户列表是否有变化
	bool _clients_change = true;

public:
	CellServer(int id )
	{
		_id = id;
		_taskServer.serverId = id;
		_pNetEvent = nullptr;
	}

	~CellServer()
	{
		CellLog::Info("CELLServer%d.~CELLServer exit begin\n", _id);
		Close();
		CellLog::Info("CELLServer%d.~CELLServer exit end\n", _id);
	}

	//void addSendTask(CellClient& pClient, netmsg_DataHeader& header)
	//{
	//	//auto task = std::make_shared<CellSToCTask>(pClient, header);//
	//	CellSToCTask* task = new CellSToCTask(pClient, header);
	//	_taskServer.addTask((CellSToCTask)task);
	//}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
	void Close()
	{
		CellLog::Info("CellServer-%d.close begin\n", _id);
		_taskServer.Close();
		_thread.Close();
		CellLog::Info("CellServer-%d.close end\n", _id);
	}

	//处理网络消息
	void OnRun(CellThread* pThead)
	{
		while (pThead->isRun())
		{
			//从缓冲队列里取出客户数据
			if (!_clientsBuff.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					pClient->serverId = _id;
					if (_pNetEvent)
						_pNetEvent->OnNetJoin(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			// 前期没有数据时，非常占线程，休眠1毫秒 ( 如果没有需要处理的客户端，就跳过)
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
			//  timestamp
			   _oldTime = CellTime::getNowInMilliSec();
				continue;
			}

			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket） 集合
			fd_set fdWrite;
			//fd_set fdExcept;
			if (_clients_change)
			{
				_clients_change = false;
				//清理集合
				FD_ZERO(&fdRead);
				//将描述符（socket）加入集合
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock < iter.second->sockfd())
					{
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else {
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}
			memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));
			//memcpy(&fdExcept, &_fdRead_bak, sizeof(fd_set));

			// nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			// 既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			// t:是0 查询没有立即返回，不阻塞 timeval t = { 0,0 };
			timeval t = { 0,1 };
			int ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);
			if (ret < 0)
			{
				CellLog::Info("CellServer.OnRun.select Error.\n");
				pThead->Exit();
				break;
			}

			ReadData(fdRead);
			WriteData(fdWrite);
			//WriteData(fdExcept);
			//if (fdExcept.fd_count>0)
			//{
			//	CellLog::Info("### fdExc= %d\n", fdExcept.fd_count);
			//}
			CheckTime();
		}
		CellLog::Info("CellServer%d.OnRun.select Error exit\n", _id);
	}

	void CheckTime()
	{
		// current  timestamp
		auto nowTime = CellTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;

		for (auto iter = _clients.begin();iter!=_clients.end();)
		{
			// Heart to check
			if (iter->second->checkHeart(dt))
			{
				if (_pNetEvent) _pNetEvent->OnNetLeave(iter->second);
				_clients_change = true;
			    delete iter->second;
				auto iterOld = iter++;
				_clients.erase(iterOld);
				continue;
			}
			//per minte to check to send data 
			//iter->second->checkSend(dt);
			iter++;
		}
	}

	void WriteData(fd_set& fdWrite)
	{
#ifdef _WIN32
		for (u_int n = 0; n < fdWrite.fd_count; n++)
		{
			auto iter = _clients.find(fdWrite.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);

					_clients.erase(iter);
				}
			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (FD_ISSET(iter->second->sockfd(), &fdWrite))
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}

	void OnClientLeave(CellClient* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetLeave(pClient);
		_clients_change = true;
		delete pClient;
	}

	void ReadData(fd_set& fdRead)
	{
#ifdef _WIN32
		for (u_int n = 0; n < fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (FD_ISSET(iter->second->sockfd(), &fdRead))
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}

	//接收数据 处理粘包 拆分包
	int RecvData(CellClient* pClient)
	{
		//  接收客户端数据
		int nLen = pClient->RecvData();
		if (nLen <= 0)
		{
			//CellLog::Info("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		// 接<收到网络数据>事件
		_pNetEvent->OnNetRecv(pClient);

		//pClient->resetDTheart();
		// 循环 判断是否有消息需处理
		while (pClient->hasMsg())
		{
			//处理网络消息
			OnNetMsg(pClient, pClient->front_msg());
			// 移除消息队列（缓冲区）最前的一条数据
			pClient->pop_front_msg();
		}
		return 0;
	}

	//响应网络消息
	virtual void OnNetMsg(CellClient* pClient, netmsg_DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	void addClient(CellClient* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_taskServer.Start();
		_thread.Start(
			// onCreate
			nullptr,
			// onRun
			[this](CellThread* pThead) {
			OnRun(pThead);
		},
			// onDestory
			[this](CellThread* pThead) {
			ClearClients();
		}
		);
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

private:
	void ClearClients()
	{
		//关闭套节字closesocket 应该用 EazyTcpServer 互责关闭
		CellLog::Info("CellServer%d.close start...\n", _id);
		for (auto iter : _clients)
		{
			delete iter.second;
		}
		_clients.clear();

		for (auto iter : _clientsBuff)
		{
			delete iter;
		}
		_clientsBuff.clear();
	}
};

#endif // !_CellServer_Hpp_
