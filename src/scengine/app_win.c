
#include "app.h"
#include "quit.h"
#include "zgtime.h"
#include "platforms.h"

#include <Windows.h>

#include <stdio.h>
#include <stdbool.h>

#define MAX_EVENTS 10

extern int main(int argc, char* argv[]);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR commandLine, int nCmdShow)
{
	return main(0, NULL);
}

int ZGAppInit(int argc, char* argv[], ZGAppHandlers* appHandlers, void* appContext)
{
	// Initialize COM for audio and font implementations
	HRESULT coInitializeResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(coInitializeResult))
	{
		fprintf(stderr, "Error: Failed to CoInitializeEx() with error %d\n", coInitializeResult);
		return -1;
	}

	// Initialize time
	(void)ZGGetNanoTicks();

	appHandlers->launchedHandler(appContext);
	
	while (true)
	{
		int eventCount = 0;
		MSG message;
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);

			if (message.message == WM_QUIT)
			{
				goto APP_TERMINATION;
			}

			eventCount++;
			if (eventCount >= MAX_EVENTS)
			{
				break;
			}
		}

		appHandlers->pollEventHandler(appContext, NULL);
		appHandlers->runLoopHandler(appContext);
	}

APP_TERMINATION:
	appHandlers->terminatedHandler(appContext);

	return 0;
}

void ZGAppSetAllowsScreenIdling(bool allowsScreenIdling)
{
	if (allowsScreenIdling)
	{
		SetThreadExecutionState(ES_CONTINUOUS);
	}
	else
	{
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
	}
}
