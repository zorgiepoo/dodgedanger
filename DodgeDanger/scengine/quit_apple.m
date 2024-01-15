
#import "quit.h"
#import "platforms.h"
#import <stdlib.h>

#if PLATFORM_OSX
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#endif

void ZGQuit(void)
{
	exit(0);
}

#if PLATFORM_OSX
void ZGSendQuitEvent(void)
{
	[NSApp terminate:nil];
}
#endif
