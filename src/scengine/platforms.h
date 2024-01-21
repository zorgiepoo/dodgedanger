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
