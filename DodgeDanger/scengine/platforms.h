

#pragma once

#if defined(__APPLE__) && defined(__MACH__)
#import <TargetConditionals.h>
#define PLATFORM_APPLE 1
#define PLATFORM_OSX TARGET_OS_OSX
#define PLATFORM_IOS TARGET_OS_IPHONE
#define PLATFORM_TVOS TARGET_OS_TV
#define PLATFORM_SIMULATOR TARGET_OS_SIMULATOR
#else
#define PLATFORM_APPLE 0
#define PLATFORM_OSX 0
#define PLATFORM_IOS 0
#define PLATFORM_TVOS 0
#define PLATFORM_SIMULATOR 0
#endif

#if defined(_WIN32)
#define PLATFORM_WINDOWS 1
#else
#define PLATFORM_WINDOWS 0
#endif

#if defined(linux)
#define PLATFORM_LINUX 1
#else
#define PLATFORM_LINUX 0
#endif
