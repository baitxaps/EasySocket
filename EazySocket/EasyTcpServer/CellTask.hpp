
#ifndef _CELL_TASK_H_

#include<thread>
#include<mutex>
#include<list>

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


class CellTaskServer
{
private:
	// task data
	std::list<CellTask*> _tasks;
	// task cache data 
	std::list<CellTask*> _tasksBuf;
	// 
	std::mutex _mutex;
	//
	std::thread* _thread;

public:
	CellTaskServer()
	{

	}

	virtual ~CellTaskServer()
	{

	}
	
	// to push  list table
	void addTask(CellTask* task)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_tasks.push_back(task);
	}

	void Start()
	{
	  // _thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
		std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
		t.detach();
	}

	// on doing function
	void OnRun()
	{
		while (true)
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
			for (auto pTask : _tasksBuf)
			{
				pTask->doTask();
				delete pTask;
			}
			_tasks.clear();
		}
	}
};

#endif // !_CELL_TASK_H_
