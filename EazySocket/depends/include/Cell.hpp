#ifndef _CELL_HPP_
#define _CELL_HPP_

// SOCKET
#ifdef _WIN32
	#define FD_SETSIZE      10240  //10k
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h> //uni std
	#include<arpa/inet.h>
	#include<string.h>
	#include<signal.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

// Customer Class
#include"MessageHeader.hpp"
//#include"CellObjectPool.hpp"
#include"CellTimestamp.hpp"
#include"CellTask.hpp"
#include"CELLLog.hpp"
//
#include<stdio.h>

//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 8192		//10240 * 5
#define SEND_BUFF_SIZE 10240	//RECV_BUFF_SZIE
#endif 


#endif