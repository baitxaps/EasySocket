#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_

#include"Cell.hpp"
class CellServer;

typedef std::shared_ptr<CellClient> CellClientPtr;

//�����¼��ӿ�
class INetEvent
{
public:
	//�ͻ��˼����¼�
	virtual void OnNetJoin(CellClientPtr& pClient) = 0;
	//�ͻ����뿪�¼�
	virtual void OnNetLeave(CellClientPtr& pClient) = 0;
	//�ͻ�����Ϣ�¼�
	virtual void OnNetMsg(CellServer* pCellServer, CellClientPtr& pClient, netmsg_DataHeader* header) = 0;
	//recv �¼�
	virtual void OnNetRecv(CellClientPtr& pClient) = 0;
private:

};
#endif