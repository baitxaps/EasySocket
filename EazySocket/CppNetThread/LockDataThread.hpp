#ifndef _LOCKDATATHREAD_HPP_
#define _LOCKDATATHREAD_HPP_

#include<list>
#include<mutex>

class LockDataThread
{
public:
	LockDataThread() {}

	~LockDataThread() {}

	void outMsgRecvQueue()
	{
		for (int i = 0; i < 1000; ++i) 
		{
			int commands = 0;
			bool c = outMsgLULProc(commands);
			if (c)
			{
				std::cout << "outMsgRecvQueue exec success." << std::endl;
			}
			else
			{
				std::cout << "outMsgRecvQueue exec,but data is empty."<<std::endl;
			}
		}
		std::cout << "outMsgRecvQueue Done." << std::endl;
	}

	bool outMsgLULProc(int& command)
	{
		std::unique_lock<std::mutex> sbg(msgMutex, std::try_to_lock);
		std::chrono::milliseconds dura(2);
		std::this_thread::sleep_for(dura);
		if (sbg.owns_lock())
		{
			if (!msgReceiveQueue.empty())
			{
				int command = msgReceiveQueue.front();
				msgReceiveQueue.pop_front();
				return true;
			}
			else {
				return false;
			}
		}
		else {
			std::cout << "do other thing..." << std::endl;
			return false;
		}
	}

	bool _outMsgLULProc(int& command)
	{
		std::lock(msgMutex, msgMutex_loc);
		std::lock_guard<std::mutex> guard1(msgMutex, std::adopt_lock);//std::adopt_lock 防止再构造
		std::lock_guard<std::mutex> guard2(msgMutex_loc, std::adopt_lock);

		//std::lock_guard<std::mutex> lock(msgMutex);
		//msgMutex.lock();

		if (!msgReceiveQueue.empty())
		{
			int command = msgReceiveQueue.front();
			msgReceiveQueue.pop_front();
		//	msgmutex.unlock();
		//	msgmutex_loc.unlock();

		//	msgmutex.unlock();
			return true;
		}
	//	msgmutex.unlock();
	//	msgmutex_loc.unlock();

	//	msgmutex.unlock();
		return false;
	}

	bool lock_guard_outMsgLULProc(int& command)
	{
		std::lock_guard<std::mutex> lock(msgMutex);
		if (!msgReceiveQueue.empty())
		{
			int command = msgReceiveQueue.front();
			msgReceiveQueue.pop_front();
			return true;
		}
		return false;
	}

	bool guard_adopt_lock_outMsgLULProc(int& command)
	{
		std::lock(msgMutex, msgMutex_loc);
		std::lock_guard<std::mutex> guard1(msgMutex, std::adopt_lock);//std::adopt_lock 防止再构造
		std::lock_guard<std::mutex> guard2(msgMutex_loc, std::adopt_lock);

		if (!msgReceiveQueue.empty())
		{
			int command = msgReceiveQueue.front();
			msgReceiveQueue.pop_front();
			return true;
		}
		return false;
	}


	bool lock_outMsgLULProc(int& command)
	{
		msgMutex.lock();
		if (!msgReceiveQueue.empty())
		{
			int command = msgReceiveQueue.front();
			msgReceiveQueue.pop_front();
			msgMutex.unlock();

			return true;
		}
		msgMutex.unlock();
		return false;
	}

	// 多个方法中，锁的锁定次序要一至，否则产生死锁
	// 耗时大的话，影响性能
	bool mult_lock_outMsgLULProc(int& command)
	{
		msgMutex.lock();
		msgMutex_loc.lock();
		if (!msgReceiveQueue.empty())
		{
			int command = msgReceiveQueue.front();
			msgReceiveQueue.pop_front();
			msgMutex.unlock();
			msgMutex_loc.unlock();
			return true;
		}
		msgMutex.unlock();
		msgMutex_loc.unlock();
		return false;
	}


	void inMsgRecvQueue() 
	{
		for (int i = 0; i < 1000; i++)
		{
			std::unique_lock<std::mutex> sbg(msgMutex, std::try_to_lock);
			if (sbg.owns_lock())
			{
				std::cout << "inMsgRecvQueue ,insert the elem :" << i << std::endl;
				msgReceiveQueue.push_back(i);
			}
			else
			{
				std::cout << "inMsgRecvQueue Do other." << i << std::endl;
			}
		}
		std::cout << "inMsgRecvQueue Done." << std::endl;
	}

	void lock_defer_inMsgRecvQueue()
	{
		for (int i = 0; i < 1000; i++)
		{
			std::cout << "inMsgRecvQueue ,insert the elem :" << i << std::endl;
			std::unique_lock<std::mutex> sbg(msgMutex, std::defer_lock);
			sbg.lock(); // 不用自己unlock
			// 处理共享代码

			sbg.unlock();
			// 处理非共享代码

			sbg.lock();
			// 处理共享代码

			//sbg.unlock();// 画蛇添足，但也可以

			msgReceiveQueue.push_back(i);
		}
		std::cout << "inMsgRecvQueue Done." << std::endl;
	}

	void unique_try_lock_inMsgRecvQueue()
	{
		for (int i = 0; i < 1000; i++)
		{
			std::unique_lock<std::mutex> sbg(msgMutex, std::defer_lock);
			if (sbg.try_lock() == true)
			{
				std::cout << "inMsgRecvQueue ,insert the elem :" << i << std::endl;
				msgReceiveQueue.push_back(i);
			}
			else
			{
				std::cout << "no catch the lock." << std::endl;
			}
		}
		std::cout << "inMsgRecvQueue Done." << std::endl;
	}


	void _inMsgRecvQueue()
	{
		for (int i = 0; i < 1000; i++)
		{
			std::cout << "inMsgRecvQueue ,insert the elem :" <<i<<std::endl;
			msgMutex.lock();
			msgReceiveQueue.push_back(i);
			msgMutex.unlock();
		}
		std::cout << "inMsgRecvQueue Done." << std::endl;
	}

private:
	std::list<int> msgReceiveQueue;
	std::mutex msgMutex;
	std::mutex msgMutex_loc;
};


#endif