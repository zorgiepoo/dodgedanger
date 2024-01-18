
#include "quit.h"
#include <Windows.h>

#include <stdlib.h>

void ZGQuit(void)
{
	exit(0);
}

void ZGSendQuitEvent(void)
{
	PostQuitMessage(0);
}
