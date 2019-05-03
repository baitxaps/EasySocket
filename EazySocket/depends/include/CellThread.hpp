#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include"CellSemaphore.hpp"

class CellThread
{
private:
	// lambda function
	typedef std::function<void(CellThread*)> EventCall;

public:
	static void Sleep(time_t dt)
	{
		std::chrono::microseconds t(dt);
		std::this_thread::sleep_for(t);
	}

	// thread start  
	void Start(EventCall onCreate = nullptr, EventCall onRun= nullptr, EventCall onDestory = nullptr)
	{
		// thread safe
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;
			if (onCreate)
				_onCreate = onCreate;
			if (onRun)
				_onRun = onRun;
			if (onDestory)
				_onDestory = onDestory;

			std::thread t(std::mem_fn(&CellThread::OnWork), this);
			t.detach();
		}
	}

	// thread close  
	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}
	}

	// exit when onWork function  
	// does't to die lock 
	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}

	//is't the thread  running 
	bool isRun()
	{
		return _isRun;
	}

protected:
	// thread on Running
	void OnWork()
	{
		if (_onCreate)
			_onCreate(this);
		if (_onRun)
			_onRun(this);
		if (_onDestory)
			_onDestory(this);

		_sem.wakeup();
	}

private:
	EventCall _onCreate;
	EventCall _onRun;
	EventCall _onDestory;
	// mutex
	std::mutex _mutex;
	//control the thread die or exit 
	CellSemaphore _sem;
	//judeg the thread  is running 
	bool _isRun = false;
};


#endif