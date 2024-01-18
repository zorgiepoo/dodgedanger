
#include "thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define SPIN_COUNT 2000

ZGThread ZGCreateThread(ZGThreadFunction function, const char *name, void *data)
{
	HANDLE threadHandle = CreateThread(NULL, 0, function, data, 0, NULL);
	if (threadHandle == NULL)
	{
		fprintf(stderr, "Error: Failed to create thread: %d\n", GetLastError());
	}
	return threadHandle;
}

void ZGWaitThread(ZGThread thread)
{
	if (WaitForSingleObjectEx(thread, INFINITE, FALSE) == WAIT_FAILED)
	{
		fprintf(stderr, "Error: Failed to WaitForSingleObjectEx()\n");
	}
	
	if (!CloseHandle(thread))
	{
		fprintf(stderr, "Error: Failed to CloseHandle()\n");
	}
}

ZGMutex ZGCreateMutex(void)
{
	CRITICAL_SECTION* mutex = calloc(1, sizeof(*mutex));
	InitializeCriticalSectionAndSpinCount(mutex, SPIN_COUNT);
	return mutex;
}

void ZGLockMutex(ZGMutex mutex)
{
	EnterCriticalSection(mutex);
}

void ZGUnlockMutex(ZGMutex mutex)
{
	LeaveCriticalSection(mutex);
}

void ZGDelay(uint32_t delayMilliseconds)
{
	Sleep(delayMilliseconds);
}
