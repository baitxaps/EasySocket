#include"EasyTcpClient.hpp"
#include<thread>


bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("�˳�cmdThread�߳�.\n");
			break;
		}
		else
		{
			printf("��֧�ֵ�����.\n");
		}
	}
}

int main()
{
	const int cCount = 2000;
	EasyTcpClient* client[cCount];

	for (int i = 0; i < cCount; i++)
	{
		if (!g_bRun) return 0;
		client[i] = new EasyTcpClient();
	}

	for (int i = 0; i < cCount; i++)
	{
		if (!g_bRun) return 0;
		client[i]->InitSocket();
		client[i]->Connect("192.168.0.106", 4568);// oip 111.67.206.137	vip:192.168.239.128 lip:192.168.0.106
	}

	/// thread start(UI thread)
	std::thread t1(cmdThread);
	t1.detach();
	Login login;
	strcpy(login.userName, "rhc");
	strcpy(login.passWord, "123456");

	while (g_bRun)
	{
		for (int i = 0; i < cCount; i++)
		{
			client[i]->SendData(&login);// ������
			client[i]->OnRun();// ��������
		}
		// ����״̬��������������
		// Sleep(1000);
	}

	for (int i = 0; i < cCount; i++)
	{
		client[i]->Close();
	}

	printf("���˳�.�������.\n");
	return 0;
}