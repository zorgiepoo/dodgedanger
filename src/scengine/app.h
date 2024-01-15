
#pragma once

#include <stdbool.h>
#include "window.h"

typedef struct
{
	ZGWindow *(*launchedHandler)(void *);
	void (*terminatedHandler)(void *);
	void (*runLoopHandler)(void *);
	void (*pollEventHandler)(void *, void *);
	void (*suspendedHandler)(void *);
} ZGAppHandlers;

int ZGAppInit(int argc, char *argv[], ZGAppHandlers *appHandlers, void *appContext);

void ZGAppSetAllowsScreenIdling(bool allowsScreenIdling);
