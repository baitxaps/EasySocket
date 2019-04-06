#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_

#include"Cell.hpp"
class CellServer;

typedef std::shared_ptr<CellClient> CellClientPtr;

//网络事件接口
class INetEvent
{
public:
	//客户端加入事件
	virtual void OnNetJoin(CellClientPtr& pClient) = 0;
	//客户端离开事件
	virtual void OnNetLeave(CellClientPtr& pClient) = 0;
	//客户端消息事件
	virtual void OnNetMsg(CellServer* pCellServer, CellClientPtr& pClient, netmsg_DataHeader* header) = 0;
	//recv 事件
	virtual void OnNetRecv(CellClientPtr& pClient) = 0;
private:

};
#endif