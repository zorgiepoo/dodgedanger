
#include "zgtime.h"

#include <windows.h>

static uint64_t getMicroTicks(void)
{
	static LARGE_INTEGER frequency;
	static LARGE_INTEGER initialTime;
	if (frequency.QuadPart == 0)
	{
		// These calls can't fail on XP or later
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&initialTime);
	}
	
	LARGE_INTEGER performanceCounter;
	QueryPerformanceCounter(&performanceCounter);

	LARGE_INTEGER elapsedTime;
	elapsedTime.QuadPart = ((performanceCounter.QuadPart - initialTime.QuadPart) * 1000000) / frequency.QuadPart;

	return (uint64_t)elapsedTime.QuadPart;
}

uint32_t ZGGetTicks(void)
{
	return (uint32_t)(getMicroTicks() / 1000.0);
}

uint64_t ZGGetNanoTicks(void)
{
	return getMicroTicks() * 1000;
}
