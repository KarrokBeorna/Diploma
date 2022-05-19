/*
* File:       Stopwatch.h
* Project:    crn
* Created:    4 Jan 2007
* Changed:    $Date: 2009/05/04 01:14:44 $
* Changed by: $Author: artem $
* Revision:   $Revision: 1.1 $
* Created by Aptile Ltd for Schlumberger, copyright Schlumberger 2007.
* http://aptile.co.uk/
*
* Description:
* A class to measure runtime CPU usage.
*
* This gets CPU time using clock(), and uses a high-precision timer for
* wall-clock times.
*
* Use the StartLap() and StopLap() methods to start and stop the clock,
* and ShowTimes( "with a title" ) to print the times to cout.
*
* Use GetTotalWallClockSeconds() and GetLapWallClockSeconds() if
* you need to read the values as float precision numbers.
*
* Example Usage:
* Stopwatch s();
* s.StartLap();
* ... do something
* s.StopLap(); // implicitely calls s.StartLap();
* s.ShowTimes();
* ... do it again
* s.StopLap(); // time since last StopLap() or StartLap()
* s.ShowTimes("The Times");
*/

#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#include <ctime>
#include <iostream>
#include <windows.h>

// #include <sys/time.h> // also see xtime in Boost for cross-platform use.

class Stopwatch
{
private:
	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1;				// ticks
	LARGE_INTEGER t2;				// ticks
	float elapsedTime;

public:
	Stopwatch()
	{
		QueryPerformanceFrequency(&frequency);
		elapsedTime = 0.0;
	};
	~Stopwatch() {};

	void StartLap()
	{
		QueryPerformanceCounter(&t1);
	};

	void StopLap()
	{
		QueryPerformanceCounter(&t2);

		// compute and print the elapsed time in sec
		elapsedTime = (t2.QuadPart - t1.QuadPart);
		elapsedTime /= frequency.QuadPart;
	};

	void Reset()
	{
		elapsedTime = 0.0;
	};

	void ShowTimes(char const * const title = 0)
	{
		if (0 != title)
		{
			std::cout << "=== " << title << " ===\n";
		}
		std::cout << "Elapsed time (seconds): " << elapsedTime << "\n";
		std::cout << std::endl;
	};


	float GetTotalWallClockSeconds()
	{
		return elapsedTime;
	}

	float GetLapWallClockSeconds()
	{
		return elapsedTime;
	}

};

#endif /*STOPWATCH_H_*/
