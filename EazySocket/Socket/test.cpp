#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
#endif
#include<iostream>
#include<functional>

#include"EasyTcpClient.hpp"
#include"CELLMsgStream.hpp"

using namespace std;
/*
����	C4996��
To disable deprecation, use _CRT_SECURE_NO_WARNINGS.See online help for details.Socket	
'gmtime': This function or variable may be unsafe.Consider using gmtime_s instead.

'inet_addr': Use inet_pton() or InetPton() instead or define _WINSOCK_DEPRECATED_NO_WARNINGS
to disable deprecated API warnings	Socket	
*/

class MyClient : public EasyTcpClient
{
public:
	//��Ӧ������Ϣ
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGOUT_RESULT:
		{
			CellReadStream r(header);
			//��ȡ��Ϣ����
			r.ReadInt16();
			//��ȡ��Ϣ����
			r.getNetCmd();
			auto n1 = r.ReadInt8();
			auto n2 = r.ReadInt16();
			auto n3 = r.ReadInt32();
			auto n4 = r.ReadFloat();
			auto n5 = r.ReadDouble();
			uint32_t n = 0;
			r.onlyRead(n);
			char name[32] = {};
			auto n6 = r.ReadArray(name, 32);
			char pw[32] = {};
			auto n7 = r.ReadArray(pw, 32);
			int ata[10] = {};
			auto n8 = r.ReadArray(ata, 10);
			CellLog::Info("<socket=%d> recv msgType��CMD_LOGOUT_RESULT\n", (int)_pClient->sockfd());
		}
		break;
		case CMD_ERROR:
		{
			CellLog::Info("<socket=%d> recv msgType��CMD_ERROR\n", (int)_pClient->sockfd());
		}
		break;
		default:
		{
			CellLog::Info("error, <socket=%d> recv undefine msgType\n", (int)_pClient->sockfd());
		}
		}
	}
private:

};


int main()
{
	CellWriteStream s(128);
	s.setNetCmd(CMD_LOGOUT);
	s.WriteInt8(1);
	s.WriteInt16(2);
	s.WriteInt32(3);
	s.WriteFloat(4.5f);
	s.WriteDouble(6.7);
	s.WriteString("client");
	
	char a[] = "ahah";
	s.WriteArray(a, strlen(a));
	int b[] = { 1,2,3,4,5 };
	s.WriteArray(b, 5);
	s.finsh();
	MyClient client;
	client.Connect("192.168.0.107", 4567);

	while (client.OnRun())
	{
		client.SendData(s.data(), s.length());
		CellThread::Sleep(10);
	}

	system("pause");
	return 0;
}

//-------------------------------------------------------
void funT()
{
	printf("funT\n");
}

int funp(int t)
{
	printf("funP\n");
	return t;
}

int funm(int t, int m)
{
	printf("funP\n");
	return m;
}

void lambda()
{
	printf("funP\n");
}

void lambdaTest() {
	WORD ver = MAKEWORD(2, 2);
	WSAData dat;
	WSAStartup(ver, &dat);

	/*
	lambda sentance express:��������ʽ ��������
	[caputure](parmas) opt-> ret��body;��
	[�ⲿ���������б�](������) ���������->����ֵ����{ ������; };

	�����б�lambda���ʽ�Ĳ����б�ϸ������lambda���ʽ�ܹ����ʵ��ⲿ�������Լ���η�����Щ������
	1) []�������κα�����
	2) [&]�����ⲿ�����������б���������Ϊ�����ں�������ʹ�ã������ò��񣩡�
	3) [=]�����ⲿ�����������б���������Ϊ�����ں�������ʹ��(��ֵ����)��
	4) [=, &foo]��ֵ�����ⲿ�����������б������������ò���foo������
	5) [bar]��ֵ����bar������ͬʱ����������������
	6) [this]����ǰ���е�thisָ�룬��lambda���ʽӵ�к͵�ǰ���Ա����ͬ���ķ���Ȩ�ޡ�
	����Ѿ�ʹ����&���� = ����Ĭ�Ϻ��д�ѡ�
	����this��Ŀ���ǿ�����lamda��ʹ�õ�ǰ��ĳ�Ա�����ͳ�Ա������

	1).capture�ǲ����б�
	2).params�ǲ�����(ѡ��)
	3).opt�Ǻ���ѡ�������mutable,exception,attribute��ѡ�
	mutable˵��lambda���ʽ���ڵĴ�������޸ı�����ı��������ҿ��Է��ʱ�����Ķ����non-const������
	exception˵��lambda���ʽ�Ƿ��׳��쳣�Լ������쳣��
	attribute�����������ԡ�
	4).ret�Ƿ���ֵ���͡�(ѡ��)
	5).body�Ǻ����塣
	*/
	int n = 10;
	std::function<int()>call;
	call = [n]() -> int {
		printf("lambda= %d\n", n);
		return 2;
	};
	int p = call();

	//function< void() > call = funT;
	//call();

	//function<int (int)> callp = funp;
	//int n =callp(0);

	//function<int(int,int)> callm = funm;
	//int m = callm(0,0);

	WSACleanup();
	system("pause");
}

//-------------------------------------------------------