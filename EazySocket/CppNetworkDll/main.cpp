#ifndef _CPP_NET_100_DLL_H_
#define _CPP_NET_100_DLL_H_

#include"EasyTcpClient.hpp"
#include"CellMsgStream.hpp"

#ifdef _WIN32
#define EXPORT_DLL _declspec(dllexport)
#else
#define EXPORT_DLL
#endif // 

extern "C"
{
	typedef void(*OnNetMsgCallBack)(void* csObj, void* data, int len);
}

class NativeTCPClient : public EasyTcpClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		if (_callBack)
			_callBack(_csObj, header, header->dataLength);
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
	/////////////////////////////Test
	EXPORT_DLL int Add(int a, int b)
	{
		return a + b;
	}

	typedef void(*CallBack1)(const char* str);

	EXPORT_DLL void TestCall1(const char* str1, CallBack1 cb)
	{
		std::string s = "Hello ";
		s += str1;
		cb(s.c_str());
	}
	/////////////////////////////CellClient

	EXPORT_DLL void* CellClient_Create(void* csObj, OnNetMsgCallBack cb, int sendSize, int recvSize)
	{
		NativeTCPClient* obj = new NativeTCPClient();
		obj->setCallBack(csObj, cb);
		obj->InitSocket(sendSize, recvSize);
		return obj;
	}

	EXPORT_DLL bool CellClient_Connect(NativeTCPClient* obj, const char* ip, unsigned short port)
	{
		if (obj && ip)
			return SOCKET_ERROR != obj->Connect(ip, port);
		return false;
	}

	EXPORT_DLL bool CellClient_OnRun(NativeTCPClient* obj)
	{
		if (obj)
			return obj->OnRun();
		return false;
	}

	EXPORT_DLL void CellClient_Close(NativeTCPClient* obj)
	{
		if (obj)
		{
			obj->Close();
			delete obj;
		}
	}

	EXPORT_DLL int CellClient_SendData(NativeTCPClient* obj, const char* data, int len)
	{
		if (obj)
		{
			return obj->SendData(data, len);
		}
		return 0;
	}

	EXPORT_DLL int CellClient_SendWriteStream(NativeTCPClient* obj, CellWriteStream* wStream)
	{
		if (obj && wStream)
		{
			wStream->finsh();
			return obj->SendData(wStream->data(), wStream->length());
		}
		return 0;
	}
	/////////////////////////////CellStream
	/////CellWriteStream

	EXPORT_DLL void* CellWriteStream_Create(int nSize)
	{
		CellWriteStream* wStream = new CellWriteStream(nSize);
		return wStream;
	}

	EXPORT_DLL bool CellWriteStream_WriteInt8(CellWriteStream* wStream, int8_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteInt16(CellWriteStream* wStream, int16_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteInt32(CellWriteStream* wStream, int32_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteInt64(CellWriteStream* wStream, int64_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteUInt8(CellWriteStream* wStream, uint8_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteUInt16(CellWriteStream* wStream, uint16_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteUInt32(CellWriteStream* wStream, uint32_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteUInt64(CellWriteStream* wStream, uint64_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteFloat(CellWriteStream* wStream, float n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteDouble(CellWriteStream* wStream, double n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteString(CellWriteStream* wStream, char* n)
	{
		if (wStream)
			return wStream->WriteString(n);
		return false;
	}

	EXPORT_DLL void CellWriteStream_Release(CellWriteStream* wStream, char* n)
	{
		if (wStream)
			delete wStream;
	}

	/////CellReadStream
	EXPORT_DLL void* CellReadStream_Create(char* data, int len)
	{
		CellReadStream* rStream = new CellReadStream(data, len);
		return rStream;
	}

	EXPORT_DLL int8_t CellReadStream_ReadInt8(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadInt8();
		return 0;
	}

	EXPORT_DLL int16_t CellReadStream_ReadInt16(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadInt16();
		return 0;
	}

	EXPORT_DLL int32_t CellReadStream_ReadInt32(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadInt32();
		return 0;
	}

	EXPORT_DLL int64_t CellReadStream_ReadInt64(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadInt64();
		return 0;
	}

	EXPORT_DLL uint8_t CellReadStream_ReadUInt8(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadUInt8();
		return 0;
	}

	EXPORT_DLL uint16_t CellReadStream_ReadUInt16(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadUInt16();
		return 0;
	}

	EXPORT_DLL uint32_t CellReadStream_ReadUInt32(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadUInt32();
		return 0;
	}

	EXPORT_DLL uint64_t CellReadStream_ReadUInt64(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadUInt64();
		return 0;
	}

	EXPORT_DLL float CellReadStream_ReadFloat(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadFloat();
		return 0.0f;
	}

	EXPORT_DLL double CellReadStream_ReadDouble(CellReadStream* rStream)
	{
		if (rStream)
			return rStream->ReadDouble();
		return 0.0;
	}

	EXPORT_DLL bool CellReadStream_ReadString(CellReadStream* rStream, char* buffer, int len)
	{
		if (rStream && buffer)
		{
			return rStream->ReadArray(buffer, len);
		}
		return false;
	}

	EXPORT_DLL uint32_t CellReadStream_OnlyReadUInt32(CellReadStream* rStream)
	{
		uint32_t len = 0;
		if (rStream)
		{
			rStream->onlyRead(len);
		}
		return len;
	}

	EXPORT_DLL void CellReadStream_Release(CellReadStream* rStream, char* n)
	{
		if (rStream)
			delete rStream;
	}
}

#endif //_CPP_NET_100_DLL_H_
