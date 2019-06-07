#ifndef _LUA_CPP_NET_100_H_
#define _LUA_CPP_NET_100_H_

#include "cocos2d.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"
#include "scripting/lua-bindings/manual/LuaBasicConversions.h"

#include"EasyTcpClient.hpp"
#include"CellMsgStream.hpp"

#define EXPORT_LUA static

class NativeTCPClient : public EasyTcpClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		if (_callBack > 0)
		{
			LuaEngine* engine = LuaEngine::getInstance();
			lua_State* L = engine->getLuaStack()->getLuaState();

			tolua_pushuserdata(L, header);
			//tolua_pushnumber(L, header->dataLength);
			lua_pushinteger(L, header->dataLength);
			engine->getLuaStack()->executeFunctionByHandler(_callBack, 2);
		}
	}

	void setCallBack(cocos2d::LUA_FUNCTION cb)
	{
		_callBack = cb;
	}
private:
	cocos2d::LUA_FUNCTION _callBack = 0;
};


////////////////////////Test
EXPORT_LUA int Add(lua_State* L)
{
	int argc = 0;
	argc = lua_gettop(L);
	if (argc == 2)
	{
		int a, b, c;
		if (luaval_to_int32(L, 1, &a))
		{
			//do something
		}
		b = lua_tointeger(L, 2);
		c = a + b;
		lua_pushinteger(L, c);
		return 1;
	}
	else {
		CCLOG("cpp Add argc != 2, argc = %d.", argc);
	}
	return 0;
}

EXPORT_LUA int SayHeiHei(lua_State* L)
{
	//const char* str = lua_tostring(L,1);
	//
	std::string s = lua_tostring(L, 1);

	s = "HeiHei " + s;

	lua_pushstring(L, s.c_str());

	return 1;
}

EXPORT_LUA int TestCall(lua_State* L)
{
	std::string s = luaL_checkstring(L, 1);

	cocos2d::LUA_FUNCTION cb = toluafix_ref_function(L, 2, 0);

	s = "HeiHei " + s;

	lua_pushstring(L, s.c_str());
	LuaEngine* engine = LuaEngine::getInstance();
	engine->getLuaStack()->executeFunctionByHandler(cb, 1);
	return 0;
}

/////////////////////////////CellClient

EXPORT_LUA void* CellClient_Create(cocos2d::LUA_FUNCTION cb, int sendSize, int recvSize)
{
	NativeTCPClient* obj = new NativeTCPClient();
	obj->setCallBack(cb);
	obj->InitSocket(sendSize, recvSize);
	return obj;
}

EXPORT_LUA bool CellClient_Connect(NativeTCPClient* obj, const char* ip, unsigned short port)
{
	if (obj && ip)
		return SOCKET_ERROR != obj->Connect(ip, port);
	return false;
}

EXPORT_LUA bool CellClient_OnRun(NativeTCPClient* obj)
{
	if (obj)
		return obj->OnRun();
	return false;
}

EXPORT_LUA void CellClient_Close(NativeTCPClient* obj)
{
	if (obj)
	{
		obj->Close();
		delete obj;
	}
}

EXPORT_LUA int CellClient_SendData(NativeTCPClient* obj, const char* data, int len)
{
	if (obj)
	{
		return obj->SendData(data, len);
	}
	return 0;
}

EXPORT_LUA int CellClient_SendWriteStream(NativeTCPClient* obj, CellWriteStream* wStream)
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
EXPORT_LUA void* CellWriteStream_Create(int nSize)
{
	CellWriteStream* wStream = new CellWriteStream(nSize);
	return wStream;
}
/////CellReadStream
EXPORT_LUA void* CellReadStream_Create(void* data)
{
	CellReadStream* rStream = new CellReadStream((netmsg_DataHeader*)data);
	return rStream;
}
/////////////////////////////CellClient LUA
EXPORT_LUA int lua_CellClient_Create(lua_State* L)
{
	cocos2d::LUA_FUNCTION cb;
	int sendSize;
	int recvSize;

	cb = toluafix_ref_function(L, 1, 0);
	sendSize = tolua_tonumber(L, 2, SEND_BUFF_SIZE);
	recvSize = tolua_tonumber(L, 3, RECV_BUFF_SIZE);

	void* obj = CellClient_Create(cb, sendSize, recvSize);

	tolua_pushuserdata(L, obj);

	return 1;
}

EXPORT_LUA int lua_CellClient_Connect(lua_State* L)
{
	NativeTCPClient* obj = nullptr;
	const char* ip = nullptr;
	unsigned short port = 0;

	obj = (NativeTCPClient*)tolua_touserdata(L, 1, nullptr);
	ip = tolua_tostring(L, 2, nullptr);
	luaval_to_ushort(L, 3, &port);

	bool b = CellClient_Connect(obj, ip, port);

	tolua_pushboolean(L, b);

	return 1;
}

EXPORT_LUA int lua_CellClient_OnRun(lua_State* L)
{
	NativeTCPClient* obj = nullptr;

	obj = (NativeTCPClient*)tolua_touserdata(L, 1, nullptr);

	bool b = CellClient_OnRun(obj);

	tolua_pushboolean(L, b);

	return 1;
}

EXPORT_LUA int lua_CellClient_Close(lua_State* L)
{
	NativeTCPClient* obj = nullptr;

	obj = (NativeTCPClient*)tolua_touserdata(L, 1, nullptr);

	CellClient_Close(obj);

	return 0;
}

EXPORT_LUA int lua_CellClient_SendData(lua_State* L)
{
	NativeTCPClient* obj = nullptr;
	const char* ip = nullptr;
	int len = 0;

	obj = (NativeTCPClient*)tolua_touserdata(L, 1, nullptr);
	ip = tolua_tostring(L, 2, nullptr);
	if (!luaval_to_int32(L, 3, &len))
	{
		CCLOG("CellClient_SendData argument #3[len] is  is not found.");
	}

	int n = CellClient_SendData(obj, ip, len);

	tolua_pushnumber(L, n);

	return 1;
}

EXPORT_LUA int lua_CellClient_SendWriteStream(lua_State* L)
{
	NativeTCPClient* obj1 = nullptr;
	CellWriteStream* obj2 = nullptr;

	obj1 = (NativeTCPClient*)tolua_touserdata(L, 1, nullptr);
	obj2 = (CellWriteStream*)tolua_touserdata(L, 2, nullptr);

	int n = CellClient_SendWriteStream(obj1, obj2);

	tolua_pushnumber(L, n);

	return 1;
}
/////////////////////////////CellStream Lua
/////CellWriteStream Lua
EXPORT_LUA int lua_CellWriteStream_Create(lua_State* L)
{
	//int nSize = lua_tointeger(L, 1);
	int nSize = tolua_tonumber(L, 1, 128);
	void* obj = CellWriteStream_Create(nSize);
	tolua_pushuserdata(L, obj);
	return 1;
}

EXPORT_LUA int lua_CellWriteStream_Release(lua_State* L)
{
	//取参数
	auto obj = (CellWriteStream*)tolua_touserdata(L, 1, nullptr);
	if (obj)
		delete obj;
	return 0;
}

template<typename T>
EXPORT_LUA int lua_CellWriteStream_Write(lua_State* L)
{
	//取参数
	auto obj = (CellWriteStream*)tolua_touserdata(L, 1, nullptr);
	T n = (T)tolua_tonumber(L, 2, 0);
	//运算
	bool b = false;
	if (obj)
		b = obj->Write(n);
	//返回结果
	tolua_pushboolean(L, b);
	return 1;
}

EXPORT_LUA int lua_CellWriteStream_WriteInt8(lua_State* L)
{
	return lua_CellWriteStream_Write<int8_t>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteInt16(lua_State* L)
{
	return lua_CellWriteStream_Write<int16_t>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteInt32(lua_State* L)
{
	return lua_CellWriteStream_Write<int32_t>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteInt64(lua_State* L)
{
	return lua_CellWriteStream_Write<int64_t>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteUInt8(lua_State* L)
{
	return lua_CellWriteStream_Write<uint8_t>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteUInt16(lua_State* L)
{
	return lua_CellWriteStream_Write<uint16_t>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteUInt32(lua_State* L)
{
	return lua_CellWriteStream_Write<uint32_t>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteUInt64(lua_State* L)
{
	return lua_CellWriteStream_Write<uint64_t>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteFloat(lua_State* L)
{
	return lua_CellWriteStream_Write<float>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteDouble(lua_State* L)
{
	return lua_CellWriteStream_Write<double>(L);
}

EXPORT_LUA int lua_CellWriteStream_WriteString(lua_State* L)
{
	//取参数
	auto obj = (CellWriteStream*)tolua_touserdata(L, 1, nullptr);
	const char* str = tolua_tostring(L, 2, nullptr);
	//运算
	bool b = false;
	if (obj)
		b = obj->WriteString(str);
	//返回结果
	tolua_pushboolean(L, b);
	return 1;
}
/////CellReadStream lua
EXPORT_LUA int lua_CellReadStream_Create(lua_State* L)
{
	auto data = tolua_touserdata(L, 1, nullptr);
	void* obj = CellReadStream_Create(data);
	tolua_pushuserdata(L, obj);
	return 1;
}

EXPORT_LUA int lua_CellReadStream_Release(lua_State* L)
{
	//取参数
	auto obj = (CellReadStream*)tolua_touserdata(L, 1, nullptr);
	if (obj)
		delete obj;
	return 0;
}

template<typename T>
EXPORT_LUA int lua_CellReadStream_Read(lua_State* L)
{
	//取参数
	auto obj = (CellReadStream*)tolua_touserdata(L, 1, nullptr);
	//运算
	T n = 0;
	if (obj)
		obj->Read(n);
	//返回结果
	tolua_pushnumber(L, n);
	return 1;
}

EXPORT_LUA int lua_CellReadStream_ReadInt8(lua_State* L)
{
	return lua_CellReadStream_Read<int8_t>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadInt16(lua_State* L)
{
	return lua_CellReadStream_Read<int16_t>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadInt32(lua_State* L)
{
	return lua_CellReadStream_Read<int32_t>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadInt64(lua_State* L)
{
	return lua_CellReadStream_Read<int64_t>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadUInt8(lua_State* L)
{
	return lua_CellReadStream_Read<uint8_t>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadUInt16(lua_State* L)
{
	return lua_CellReadStream_Read<uint16_t>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadUInt32(lua_State* L)
{
	return lua_CellReadStream_Read<uint32_t>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadUInt64(lua_State* L)
{
	return lua_CellReadStream_Read<uint64_t>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadFloat(lua_State* L)
{
	return lua_CellReadStream_Read<float>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadDouble(lua_State* L)
{
	return lua_CellReadStream_Read<double>(L);
}

EXPORT_LUA int lua_CellReadStream_ReadString(lua_State* L)
{
	//取参数
	auto obj = (CellReadStream*)tolua_touserdata(L, 1, nullptr);
	//运算
	std::string n;
	if (obj)
		obj->ReadString(n);
	//返回结果
	//tolua_pushstring(L, n.c_str());
	tolua_pushcppstring(L, n);
	return 1;
}

////////////
EXPORT_LUA int lua_CppNet100_Test_register(lua_State* L)
{
	lua_register(L, "Add", Add);
	lua_register(L, "SayHi", SayHeiHei);
	lua_register(L, "TestCall", TestCall);
	return 0;
}

EXPORT_LUA int lua_CppNet100_Cell_register(lua_State* L)
{
	tolua_open(L);
	tolua_module(L, nullptr, 0);
	tolua_beginmodule(L, nullptr);
	tolua_function(L, "CellClient_Create", lua_CellClient_Create);
	tolua_function(L, "CellClient_Connect", lua_CellClient_Connect);
	tolua_function(L, "CellClient_OnRun", lua_CellClient_OnRun);
	tolua_function(L, "CellClient_Close", lua_CellClient_Close);
	tolua_function(L, "CellClient_SendData", lua_CellClient_SendData);
	tolua_function(L, "CellClient_SendWriteStream", lua_CellClient_SendWriteStream);
	////
	tolua_function(L, "CellWriteStream_Create", lua_CellWriteStream_Create);
	tolua_function(L, "CellWriteStream_WriteInt8", lua_CellWriteStream_WriteInt8);
	tolua_function(L, "CellWriteStream_WriteInt16", lua_CellWriteStream_WriteInt16);
	tolua_function(L, "CellWriteStream_WriteInt32", lua_CellWriteStream_WriteInt32);
	tolua_function(L, "CellWriteStream_WriteInt64", lua_CellWriteStream_WriteInt64);
	tolua_function(L, "CellWriteStream_WriteUInt8", lua_CellWriteStream_WriteUInt8);
	tolua_function(L, "CellWriteStream_WriteUInt16", lua_CellWriteStream_WriteUInt16);
	tolua_function(L, "CellWriteStream_WriteUInt32", lua_CellWriteStream_WriteUInt32);
	tolua_function(L, "CellWriteStream_WriteUInt64", lua_CellWriteStream_WriteUInt64);
	tolua_function(L, "CellWriteStream_WriteFloat", lua_CellWriteStream_WriteFloat);
	tolua_function(L, "CellWriteStream_WriteDouble", lua_CellWriteStream_WriteDouble);
	tolua_function(L, "CellWriteStream_WriteString", lua_CellWriteStream_WriteString);
	tolua_function(L, "CellWriteStream_Release", lua_CellWriteStream_Release);
	////
	tolua_function(L, "CellReadStream_Create", lua_CellReadStream_Create);
	tolua_function(L, "CellReadStream_ReadInt8", lua_CellReadStream_ReadInt8);
	tolua_function(L, "CellReadStream_ReadInt16", lua_CellReadStream_ReadInt16);
	tolua_function(L, "CellReadStream_ReadInt32", lua_CellReadStream_ReadInt32);
	tolua_function(L, "CellReadStream_ReadInt64", lua_CellReadStream_ReadInt64);
	tolua_function(L, "CellReadStream_ReadUInt8", lua_CellReadStream_ReadUInt8);
	tolua_function(L, "CellReadStream_ReadUInt16", lua_CellReadStream_ReadUInt16);
	tolua_function(L, "CellReadStream_ReadUInt32", lua_CellReadStream_ReadUInt32);
	tolua_function(L, "CellReadStream_ReadUInt64", lua_CellReadStream_ReadUInt64);
	tolua_function(L, "CellReadStream_ReadFloat", lua_CellReadStream_ReadFloat);
	tolua_function(L, "CellReadStream_ReadDouble", lua_CellReadStream_ReadDouble);
	tolua_function(L, "CellReadStream_ReadString", lua_CellReadStream_ReadString);
	tolua_function(L, "CellReadStream_Release", lua_CellReadStream_Release);
	////
	tolua_endmodule(L);
	return 0;
}

EXPORT_LUA int lua_CppNetwork_register(lua_State* L)
{
	lua_CppNet100_Test_register(L);
	lua_CppNet100_Cell_register(L);
	return 0;
}

#endif // !_LUA_CPP_NET_100_H_
