/*
 MIT License

 Copyright (c) 2024 Mayur Pawashe

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

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
