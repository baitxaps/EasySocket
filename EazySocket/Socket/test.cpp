#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
#endif
#include<iostream>
#include<windows.h>
#include<WinSock2.h>
#include<functional>

//#pragma comment(lib,"ws2_32.lib")
using namespace std;

void funT()
{
	printf("funT\n");
}

int funp(int t)
{
	printf("funP\n");
	return t;
}

int funm(int t,int m)
{
	printf("funP\n");
	return m;
}

void lambda()
{
	printf("funP\n");
}

int main()
{
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
	int n=10;
	std::function<int()>call;
	call = [n]() -> int {
		printf("lambda= %d\n",n);
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
	return 0;
}
