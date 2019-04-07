#ifndef _CellServer_Hpp_
#define _CellServer_Hpp_

#include"Cell.hpp"
#include"INetEvent.hpp"
#include"CellClient.hpp"

#include<vector>
#include<map>

// network message task class  to send 
class CellSToCTask :public CellTask
{
private:
	CellClientPtr _pClient;
	DataHeaderPtr	_pHeader;

public:
	CellSToCTask(CellClientPtr pClient, DataHeaderPtr& pHeader)
	{
		_pClient = pClient;
		_pHeader = pHeader;
	}

	virtual void doTask()
	{
		_pClient->SendData(_pHeader);
		//delete _pHeader;
	}
};

// network msg for handling class
class CellServer
{
private:
	SOCKET _sock;
	//��ʽ�ͻ�����
	std::map<SOCKET, CellClientPtr> _clients;
	//����ͻ�����
	std::vector<CellClientPtr> _clientsBuff;
	//������е���
	std::mutex _mutex;
	std::thread _thread;
	//�����¼�����
	INetEvent* _pNetEvent;
	//
	CellTaskServer _taskServer;
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
	}

	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	void addSendTask(CellClientPtr& pClient, DataHeaderPtr& header)
	{
		auto task = std::make_shared<CellSToCTask>(pClient, header);//CellSToCTask* task = new CellSToCTask(pClient, header);
		_taskServer.addTask((CellTaskPtr)task);
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				//delete iter.second;
			}
			//�ر��׽���closesocket
			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				close(iter.second->sockfd());
				delete iter.second;
			}
			//�ر��׽���closesocket
			close(_sock);
#endif
			_clients.clear();
		}
	}

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//����������Ϣ
	//���ݿͻ�socket fd_set
	fd_set _fdRead_bak;
	//�ͻ��б��Ƿ��б仯
	bool _clients_change;
	SOCKET _maxSock;
	void OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			//�ӻ��������ȡ���ͻ�����
			if (_clientsBuff.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			// ǰ��û������ʱ���ǳ�ռ�̣߳�����1���� ( ���û����Ҫ����Ŀͻ��ˣ�������)
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
			//  timestamp
			   _old_time = CellTime::getNowInMilliSec();
				continue;
			}

			//�������׽��� BSD socket
			fd_set fdRead;//��������socket�� ����
						  //������
			FD_ZERO(&fdRead);
			if (_clients_change)
			{
				_clients_change = false;
				//����������socket�����뼯��
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

			// nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			// ���������ļ����������ֵ+1 ��Windows�������������д0
			// t:��0 ��ѯû���������أ������� timeval t = { 0,0 };
			timeval t = { 0,1 };
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, &t);
			if (ret < 0)
			{
				printf("select���������\n");
				Close();
				return;
			}

			//else if (ret == 0)
			//{
			//	continue;
			//}
			ReadData(fdRead);
			CheckTime();
		}
	}

	// old  timestamp
	time_t _old_time = CellTime::getNowInMilliSec();
	void CheckTime()
	{
		// current  timestamp
		auto nowTime = CellTime::getNowInMilliSec();
		auto dt = nowTime - _old_time;
		_old_time = nowTime;

		for (auto iter = _clients.begin();iter!=_clients.end();)
		{
			// Heart to check
			if (iter->second->checkHeart(dt))
			{
				if (_pNetEvent) _pNetEvent->OnNetLeave(iter->second);
				_clients_change = true;
			  // delete iter->second;
				auto iterOld = iter++;
				_clients.erase(iterOld);
				continue;
			}
			// send data to check
			iter->second->checkSend(dt);
			iter++;
		}
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
					if (_pNetEvent)
					{
						_pNetEvent->OnNetLeave(iter->second);
					}
					_clients_change = true;
					closesocket(iter->first);
				//	delete iter->second;
					_clients.erase(iter);
				}
			}
			else {
				printf("error. iter != _clients.end()...\n");
			}
		}
#else
		std::vector<CellClientPtr> temp;
		for (auto iter : _clients)
		{
			if (FD_ISSET(iter.second->sockfd(), &fdRead))
			{
				if (-1 == RecvData(iter.second))
				{
					if (_pNetEvent)
					{
						_pNetEvent->OnNetLeave(iter.second);
					}
					_clients_change = true;
					close(iter->first);
					temp.push_back(iter.second);
				}
			}
		}
		for (auto pClient : temp)
		{
			_clients.erase(pClient->sockfd());
			delete pClient;
		}
#endif
	}

	//�������� ����ճ�� ��ְ�
	int RecvData(CellClientPtr pClient)
	{
		// ������
		char *_szRecv = pClient->msgBuf() + pClient->getLastPos();
		//  ���տͻ�������
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SZIE - pClient->getLastPos(), 0);

		_pNetEvent->OnNetRecv(pClient);

		if (nLen <= 0)
		{
			//printf("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
			return -1;
		}

		//pClient->resetDTheart();

		//����ȡ�������ݿ�������Ϣ������ memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);

		//��Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//�ж���Ϣ�����������ݳ��ȴ�����Ϣͷnetmsg_DataHeader����
		while (pClient->getLastPos() >= sizeof(netmsg_DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			netmsg_DataHeader* header = (netmsg_DataHeader*)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (pClient->getLastPos() >= header->dataLength)
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = pClient->getLastPos() - header->dataLength;
				//����������Ϣ
				OnNetMsg(pClient, header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//��Ϣ������������β��λ��ǰ��
				pClient->setLastPos(nSize);
			}
			else {
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(CellClientPtr pClient, netmsg_DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	void addClient(CellClientPtr pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
		//  std::thread t(std::mem_fun(&CellServer::OnRun), this);
		_taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
};

#endif // !_CellServer_Hpp_
