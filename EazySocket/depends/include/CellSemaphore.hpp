#ifndef _CELL_SEMAPHOER_HPP_
#define _CELL_SEMAPHOER_HPP_

#include<chrono>
#include<thread>
#include<mutex>
#include<condition_variable>

class CellSemaphore
{
public:
	CellSemaphore()
	{

	}

	~CellSemaphore()
	{

	}

	// thread block
	void wait()
	{
		std::unique_lock<std::mutex>lock(_mutex);
		if (--_wait < 0)
		{
			_cv.wait(lock, [this] ()->bool {
				return _wakeup > 0;
			});
			--_wakeup;
		}
	}

	// thread wakeup
	void wakeup()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (++_wait <= 0) {
			++_wakeup;
			_cv.notify_one();
		}
	}

private:
	std::mutex _mutex;
	// block variable
	std::condition_variable _cv;
	// wait for count 
	int _wait = 0;
	// wakeup for count
	int _wakeup = 0;
};


#endif