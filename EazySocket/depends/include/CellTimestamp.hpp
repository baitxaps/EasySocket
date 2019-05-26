#ifndef _CELL_TIMESTAMP_HPP_
#define _CELL_TIMESTAMP_HPP_
//#include <windows.h>
#include<chrono>
using namespace std::chrono;

class CellTime
{
public:
	// get the current  time. millisec
	static time_t  getNowInMilliSec()
	{
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}
};

class CellTimestamp
{
public:
	CellTimestamp()
    {
        //QueryPerformanceFrequency(&_frequency);
        //QueryPerformanceCounter(&_startCount);
		update();
    }
    ~CellTimestamp()
    {}

    void update()
    {
        //QueryPerformanceCounter(&_startCount);
		_begin = high_resolution_clock::now();
    }
    /**
    *   获取当前秒
    */
    double getElapsedSecond()
    {
        return  getElapsedTimeInMicroSec() * 0.000001;
    }
    /**
    *   获取毫秒
    */
    double getElapsedTimeInMilliSec()
    {
        return this->getElapsedTimeInMicroSec() * 0.001;
    }
    /**
    *   获取微妙
    */
    long long getElapsedTimeInMicroSec()
    {
		/*
        LARGE_INTEGER endCount;
        QueryPerformanceCounter(&endCount);

        double  startTimeInMicroSec =   _startCount.QuadPart * (1000000.0 / _frequency.QuadPart);
        double  endTimeInMicroSec   =   endCount.QuadPart * (1000000.0 / _frequency.QuadPart);

        return  endTimeInMicroSec - startTimeInMicroSec;
		*/
		
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
    }
protected:
    //LARGE_INTEGER   _frequency;
    //LARGE_INTEGER   _startCount;
	time_point<high_resolution_clock> _begin;
};

#endif 