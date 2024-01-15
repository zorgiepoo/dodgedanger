
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "texture.h"

void initFontFromFile(const char *filePath, int pointSize);
void initFontWithName(const char *name, int pointSize);

TextureData createTextData(const char* string);

#ifdef __cplusplus
}
#endif
