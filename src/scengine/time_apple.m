
#include "zgtime.h"
#import <Foundation/Foundation.h>
#include <mach/mach_time.h>

uint32_t ZGGetTicks(void)
{
	return (uint32_t)(ZGGetNanoTicks() / 1000000.0);
}

uint64_t ZGGetNanoTicks(void)
{
	static mach_timebase_info_data_t info;
	static double initialTime = 0.0;
	static bool initializedTime;
	if (!initializedTime)
	{
		mach_timebase_info(&info);
		initialTime = (1.0 * mach_absolute_time() * info.numer / info.denom);
		
		initializedTime = true;
	}
	
	return (uint64_t)((1.0 * mach_absolute_time() * info.numer / info.denom) - initialTime);
}
