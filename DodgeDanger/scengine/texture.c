
#include "texture.h"
#include "renderer.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

TextureObject loadTextureFromData(Renderer *renderer, TextureData textureData)
{
	return textureFromPixelData(renderer, textureData.pixelData, textureData.width, textureData.height, textureData.pixelFormat);
}

TextureObject loadTexture(Renderer *renderer, const char *filePath)
{
	TextureData textureData = loadTextureData(filePath);
	
	TextureObject texture = loadTextureFromData(renderer, textureData);
	
	freeTextureData(textureData);
	
	return texture;
}

TextureData copyTextureData(TextureData textureData)
{
	TextureData copyData = textureData;
#if TEXTURE_DATA_HAS_CONTEXT
	copyData.context = NULL;
#endif
	
	size_t numBytes = textureData.width * textureData.height * 4;
	copyData.pixelData = calloc(1, numBytes);
	assert(copyData.pixelData != NULL);
	memcpy(copyData.pixelData, textureData.pixelData, numBytes);
	
	return copyData;
}
