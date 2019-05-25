#ifndef _CPP_NETDLL_H_

#include<string>
#include"EasyTcpClient.hpp"

#ifdef _WIN32
	#define EXPORT_DLL _declspec(dllexport)
#else
	#define EXPORT_DLL
#endif 

extern "C"
{
	typedef void(*OnNetMsgCallBack)(void* csObj, void* data, int len);
}

class NativeTCPClient :public EasyTcpClient
{
public:
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		if (_callBack)
		{
			_callBack(_csObj, header, header->dataLength);
		}
	}

	void setCallBack(void* csObj, OnNetMsgCallBack cb)
	{
		_csObj = csObj;
		_callBack = cb;
	}

private:
	void* _csObj = nullptr;
	OnNetMsgCallBack _callBack = nullptr;
};


extern "C"
{
	EXPORT_DLL int Add(int a, int b)
	{
		return a + b;
	}

	typedef void(*CallBack1)(const char* str);
	EXPORT_DLL void TestCall(const char * str1, CallBack1 cb)
	{
		std::string s = "Hello ";
		s += str1;
		cb(s.c_str());
	}
	////////////////////////////////////////

	EXPORT_DLL void* CellClient_Create(void* csObj, OnNetMsgCallBack cb)
	{
		NativeTCPClient* pClient = new NativeTCPClient();
		pClient->setCallBack(csObj, cb);
		return pClient;
	}

	EXPORT_DLL bool CellClient_Connect(NativeTCPClient* pClient, const char* ip, short port)
	{
		if (pClient && ip)
			return  SOCKET_ERROR != pClient->Connect(ip, port);
		return false;
	}

	EXPORT_DLL bool CellClient_OnRun(NativeTCPClient* pClient)
	{
		if (pClient)
			return pClient->OnRun();
		return false;
	}

	EXPORT_DLL void CellClient_Close(NativeTCPClient* pClient)
	{
		if (pClient)
		{
			pClient->OnRun();
			delete pClient;
		}
	}

	EXPORT_DLL int CellClient_SendData(NativeTCPClient* pClient, const char* data, int len)
	{
		if (pClient)
		{
			return pClient->SendData(data, len);
		}
		return 0;
	}
}

#endif // !_CPP_NETDLL_H_