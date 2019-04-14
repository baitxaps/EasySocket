#ifndef _CELL_SEMAPHOER_HPP_
#define _CELL_SEMAPHOER_HPP_

#include<chrono>
#include<thread>

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
		_isWaitExit = true;
		while (_isWaitExit)
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
		}
	}

	// thread wakeup
	void wakeup()
	{
		if (_isWaitExit)
			_isWaitExit = false;
		else
			printf("CellSemaphore wakeup error.\n");
	}

private:
	bool _isWaitExit = false;
};


#endif