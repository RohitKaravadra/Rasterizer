#pragma once
#include <chrono>
#include <iostream>

// template to create a chrono clock timer
// - Clock = high_resolution_clock or steady_clock or system_clock
template <typename Clock>
class ChronoClock
{
	using TimeType = typename Clock::time_point; // setting time

	TimeType start; // clock start time
	TimeType end; //  clock end time
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
			start = std::chrono::high_resolution_clock::now();
	}

	// stop clock if enabled
	void Stop()
	{
		if (enable)
			end = std::chrono::high_resolution_clock::now();
	}

	// print clock time in miliseconds if enabled
	void Print() const
	{
		if (enable)
		{
			auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			std::cout << "Time : " << diff.count() << " ms" << std::endl;
		}
	}
};
