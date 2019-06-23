#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include"CellSemaphore.hpp"
#include <functional>

class CellThread
{
public:
    static void Sleep(time_t dt)
    {
        std::chrono::milliseconds t(dt);
        std::this_thread::sleep_for(t);
    }
private:
    typedef std::function<void(CellThread*)> EventCall;
public:
    //starting a thread
    void Start(
               EventCall onCreate = nullptr,
               EventCall onRun = nullptr,
               EventCall onDestory = nullptr)
    {
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
            
            //thread
            std::thread t(std::mem_fn(&CellThread::OnWork), this);
            t.detach();
        }
    }
    
    //close the thread
    void Close()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_isRun)
        {
            _isRun = false;
            _sem.wait();
        }
    }
    //exit in the work function
    //wait for don't need to use semaphore to jam
    //if you use a clog
    void Exit()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_isRun)
        {
            _isRun = false;
        }
    }
    
    //whether the thread is up and running state
    bool isRun()
    {
        return _isRun;
    }
protected:
    //Thread the runtime of work function
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
    //Change the data in different threads need to lock
    std::mutex _mutex;
    //control the thread die or exit
    CellSemaphore _sem;
    //judeg the thread  is running
    bool    _isRun = false;
};


#endif // !_CELL_THREAD_HPP_
