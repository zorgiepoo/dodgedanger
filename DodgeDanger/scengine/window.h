
#pragma once

#include "platforms.h"

#if PLATFORM_IOS
#include "touch.h"
#else
#include "keyboard.h"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef void ZGWindow;
typedef enum
{
	ZGWindowEventTypeResize,
	ZGWindowEventTypeFocusGained,
	ZGWindowEventTypeFocusLost,
	ZGWindowEventTypeShown,
	ZGWindowEventTypeHidden,
#if PLATFORM_WINDOWS
	ZGWindowEventDeviceConnected
#endif
} ZGWindowEventType;

typedef struct
{
	int32_t width;
	int32_t height;
	
	ZGWindowEventType type;
} ZGWindowEvent;

ZGWindow *ZGCreateWindow(const char *windowTitle, int32_t windowWidth, int32_t windowHeight, bool *fullscreenFlag);
void ZGDestroyWindow(ZGWindow *window);

bool ZGWindowHasFocus(ZGWindow *window);

#if PLATFORM_WINDOWS
void ZGSetWindowMinimumSize(ZGWindow *window, int32_t minWidth, int32_t minHeight);
void ZGShowWindow(ZGWindow* window);
#endif

void ZGGetWindowSize(ZGWindow *window, int32_t *width, int32_t *height);

#if !PLATFORM_IOS
void ZGSetWindowEventHandler(ZGWindow *window, void *context, void (*windowEventHandler)(ZGWindowEvent, void *));
#endif

#if PLATFORM_LINUX
bool ZGWindowIsFullscreen(ZGWindow *window);
#endif

#if PLATFORM_IOS
void ZGSetTouchEventHandler(ZGWindow *window, void *context, void (*touchEventHandler)(ZGTouchEvent, void *));

#if PLATFORM_TVOS
void ZGInstallMenuGesture(ZGWindow *window);
void ZGUninstallMenuGesture(ZGWindow *window);
#else
void ZGInstallTouchGestures(ZGWindow *window);
void ZGUninstallTouchGestures(ZGWindow *window);
#endif

#else
void ZGSetKeyboardEventHandler(ZGWindow *window, void *context, void (*keyboardEventHandler)(ZGKeyboardEvent, void *));
#endif

#if PLATFORM_LINUX
void ZGPollWindowAndInputEvents(ZGWindow *window, const void *systemEvent);
#endif

void *ZGWindowHandle(ZGWindow *window);
