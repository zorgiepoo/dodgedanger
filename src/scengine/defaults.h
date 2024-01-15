
#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "platforms.h"

typedef struct
{
#if !PLATFORM_APPLE
	FILE *file;
#endif
} Defaults;

Defaults userDefaultsForReading(const char *defaultsName);
Defaults userDefaultsForWriting(const char *defaultsName);

void closeDefaults(Defaults defaults);

bool readDefaultKey(Defaults defaults, const char *key, char *valueBuffer, size_t maxValueSize);
int readDefaultIntKey(Defaults defaults, const char *key, int defaultValue);
bool readDefaultBoolKey(Defaults defaults, const char *key, bool defaultValue);

void writeDefaultIntKey(Defaults defaults, const char *key, int value);
void writeDefaultStringKey(Defaults defaults, const char *key, const char *value);

#if PLATFORM_OSX
void getDefaultUserName(char *defaultUserName, int maxLength);
#endif
