#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_

#include"Cell.hpp"
#include"CellClient.hpp"

class CellServer;

typedef std::shared_ptr<CellClient> CellClientPtr;

//�����¼��ӿ�
class INetEvent
{
public:
	//�ͻ��˼����¼�
	//virtual void OnNetJoin(CellClientPtr& pClient) = 0;
	virtual void OnNetJoin(CellClient* pClient) = 0;
	//�ͻ����뿪�¼�
	virtual void OnNetLeave(CellClient* pClient) = 0;
	//�ͻ�����Ϣ�¼�
	virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, netmsg_DataHeader* header) = 0;
	//recv �¼�
	virtual void OnNetRecv(CellClient* pClient) = 0;
private:

};
#endif