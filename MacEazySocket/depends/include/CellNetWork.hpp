﻿#ifndef _CELL_NETWORK_HPP_
#define _CELL_NETWORK_HPP_

#include"Cell.hpp"

class CellNetWork
{
private:
	CellNetWork()
	{
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

#ifndef _WIN32
		// 忽略异常信号，默认情况会导致进程终止
		//if (signal(SIGPIPE,SIG_IGN)== SIG_ERR)
		//{
		//	return (1);
		//}
		signal(SIGPIPE, SIG_IGN);
#endif 
	}

	~CellNetWork()
	{
#ifdef _WIN32
		//清除Windows socket环境
		WSACleanup();
#endif
	}

public:
	static void Init()
	{
		static CellNetWork obj;
	}
};



#endif