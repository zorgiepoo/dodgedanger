
#pragma once

#include "platforms.h"

void ZGQuit(void);

#if !PLATFORM_IOS
void ZGSendQuitEvent(void);
#endif
