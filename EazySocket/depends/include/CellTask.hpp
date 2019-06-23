
#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include<thread>
#include<mutex>
#include<list>
#include"CellSemaphore.hpp"
#include"CellThread.hpp"

class CellTaskServer
{
public:
	int serverId = -1;// for test

private:
	typedef std::function<void()> CellTask;
private:
	// task data
	std::list<CellTask> _tasks;//std::list<CellTaskPtr>_tasks;
	// task cache data 
	std::list<CellTask> _tasksBuf;//std::list<CellTaskPtr>_tasksBuf;
	// 
	std::mutex _mutex;
	//
	CellThread _thread;
public:
	// to push  list table
	void addTask(CellTask task)
	//void addTask(CellTaskPtr& task)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_tasks.push_back(task);
	}

	void Start()
	{
		_thread.Start(nullptr, [this](CellThread* pThread) {
			OnRun(pThread);
		},nullptr);
	}

	void Close()
	{
		//CellLog::Info("CellTaskServer-%d.close begin\n", serverId);
		_thread.Close();
		//CellLog::Info("CellTaskServer-%d.close end\n", serverId);
	}

protected:
	// on doing function
	void OnRun(CellThread* pThread)
	{
		while (pThread->isRun())
		{
			if (!_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}

			if (_tasks.empty())
			{
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			// To do task... 
			for (auto pTask : _tasks)//for (auto pTask : _tasksBuf)
			{
				pTask();
			}
			_tasks.clear();
		}

		// do for cache queue task
		for (auto pTask : _tasksBuf)
		{
			pTask();
		}
		//CellLog::Info("CellTaskServerId %d.OnRun...\n", serverId);
	}
};

#endif // !_CELL_TASK_H_
