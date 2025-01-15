#include "Includes.h"

void Test()
{
	ChronoClock<std::chrono::high_resolution_clock, std::micro> clock;

	matrix a, b, c;

	for (int i = 0; i < 16; i++)
	{
		a[i] = RandomNumberGenerator::getInstance().getRandomFloat(15.f, 60.f);
		b[i] = RandomNumberGenerator::getInstance().getRandomFloat(15.f, 60.f);
	}

	clock.Start();
	c = a.mul(b);
	clock.Stop();
	clock.Print();

	clock.Start();
	c = a.mul_new(b);
	clock.Stop();
	clock.Print();
}