#pragma once
#include <chrono>
#include <iostream>
#include <ostream>

// template to create a chrono clock timer
// - Clock = high_resolution_clock or steady_clock or system_clock
template <typename Clock = std::chrono::steady_clock, typename Type = std::milli>
class ChronoClock
{
	Clock::time_point start; // clock start time
	Clock::time_point end; //  clock end time
public:
	bool enable; //enables or diables clock

	ChronoClock()
	{
		enable = true;
	}

	// start clock if enabled
	void Start()
	{
		if (enable)
			start = Clock::now();
	}

	// stop clock if enabled
	void Stop()
	{
		if (enable)
			end = Clock::now();
	}

	// print time if enabled
	void Print()
	{
		if (enable)
		{
			auto diff = std::chrono::duration<double, Type>(end - start);
			std::cout << "Time : " << diff.count() << std::endl;
		}
	}

	// output operator overload
	friend std::ostream& operator<<(std::ostream& _os, const ChronoClock& _timer)
	{
		if (_timer.enable)
		{
			auto diff = std::chrono::duration<double,Type>(_timer.end - _timer.start);
			_os << diff.count();
		}

		return _os;
	}
};
