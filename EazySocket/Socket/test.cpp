#define WIN32_LEAN_AND_MEAN
#include<iostream>


 
#include<windows.h>
#include<WinSock2.h>

//#pragma comment(lib,"ws2_32.lib")

using namespace std;

int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSAData dat;
	WSAStartup(ver, &dat);


	WSACleanup();
	system("pause");
	return 0;
}