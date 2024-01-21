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

#import "defaults.h"
#import <Foundation/Foundation.h>

Defaults userDefaultsForReading(const char *defaultsName)
{
	Defaults defaults = {};
	return defaults;
}

Defaults userDefaultsForWriting(const char *defaultsName)
{
	Defaults defaults = {};
	return defaults;
}

void closeDefaults(Defaults defaults)
{
}

bool readDefaultKey(Defaults defaults, const char *key, char *valueBuffer, size_t maxValueSize)
{
	NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
	NSString *valueString = [userDefaults stringForKey:[NSString stringWithUTF8String:key]];
	if (valueString == nil)
	{
		return false;
	}
	
	return (bool)[valueString getCString:valueBuffer maxLength:maxValueSize encoding:NSUTF8StringEncoding];
}

int readDefaultIntKey(Defaults defaults, const char *key, int defaultValue)
{
	NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
	id object = [userDefaults objectForKey:[NSString stringWithUTF8String:key]];
	if (object != nil && [object isKindOfClass:[NSNumber class]])
	{
		return [object intValue];
	}
	
	return defaultValue;
}

bool readDefaultBoolKey(Defaults defaults, const char *key, bool defaultValue)
{
	NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
	id object = [userDefaults objectForKey:[NSString stringWithUTF8String:key]];
	if (object != nil && [object isKindOfClass:[NSNumber class]])
	{
		return [object boolValue];
	}
	
	return defaultValue;
}

void writeDefaultIntKey(Defaults defaults, const char *key, int value)
{
	NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
	[userDefaults setInteger:value forKey:[NSString stringWithUTF8String:key]];
}

void writeDefaultStringKey(Defaults defaults, const char *key, const char *value)
{
	NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
	[userDefaults setObject:[NSString stringWithUTF8String:value] forKey:[NSString stringWithUTF8String:key]];
}

#if PLATFORM_OSX
void getDefaultUserName(char *defaultUserName, int maxLength)
{
	NSString *fullUsername = NSFullUserName();
	if (fullUsername)
	{
		NSUInteger spaceIndex = [fullUsername rangeOfString:@" "].location;
		if (spaceIndex != NSNotFound)
		{
			strncpy(defaultUserName, [[fullUsername substringToIndex:spaceIndex] UTF8String], maxLength);
		}
		else
		{
			strncpy(defaultUserName, [fullUsername UTF8String], maxLength);
		}
	}
}
#endif
