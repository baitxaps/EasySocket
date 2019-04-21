
#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include<thread>
#include<mutex>
#include<list>
#include"CellSemaphore.hpp"
#include"CellThread.hpp"

//#include"CellLog.hpp"

// Task type for base class
class CellTask
{
private:

public:
	CellTask()
	{

	}

	virtual ~CellTask() 
	{

	}

	//Task Exec 
	virtual void doTask()
	{

	}
};

typedef std::shared_ptr<CellTask> CellTaskPtr;
class CellTaskServer
{
public:
	int serverId = -1;// for test

private:
	typedef std::function<void()> CellTask;
private:
	// task data
	std::list<CellTaskPtr>_tasks;//std::list<CellTask*> _tasks;
	// task cache data 
	std::list<CellTaskPtr>_tasksBuf;//std::list<CellTask*> _tasksBuf;
	// 
	std::mutex _mutex;
	//
	CellThread _thread;
public:
	CellTaskServer()
	{
	}

	virtual ~CellTaskServer()
	{
	}

	// to push  list table
	//void addTask(CellTask* task)
	void addTask(CellTaskPtr& task)
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
				pTask->doTask();
			//	delete pTask;
			}
			_tasks.clear();
		}

		// do for cache queue task
		for (auto pTask : _tasksBuf)
		{
			pTask->doTask();
			//	delete pTask;
		}

		//CellLog::Info("CellTaskServerId %d.OnRun...\n", serverId);
	}

};

#endif // !_CELL_TASK_H_
