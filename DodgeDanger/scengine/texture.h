
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "renderer_types.h"
#include "platforms.h"

#define TEXTURE_DATA_HAS_CONTEXT !PLATFORM_WINDOWS

typedef struct
{
	uint8_t* pixelData;
#if TEXTURE_DATA_HAS_CONTEXT
	void* context;
#endif
	int32_t width;
	int32_t height;
	// Pixel format is guaranteed to have 4 8-bit components including alpha component at the end
	PixelFormat pixelFormat;
} TextureData;

TextureData loadTextureData(const char* filePath);
TextureData copyTextureData(TextureData textureData);
void freeTextureData(TextureData textureData);

TextureObject loadTextureFromData(Renderer* renderer, TextureData textureData);
TextureObject loadTexture(Renderer* renderer, const char* filePath);

#ifdef __cplusplus
}
#endif
