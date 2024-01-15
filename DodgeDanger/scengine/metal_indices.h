
#ifndef metal_indices_h
#define metal_indices_h

typedef enum
{
	METAL_BUFFER_VERTICES_INDEX = 0,
	METAL_BUFFER_MODELVIEW_PROJECTION_INDEX = 1,
	METAL_BUFFER_COLOR_INDEX = 2,
	METAL_BUFFER_TEXTURE_COORDINATES_INDEX = 3,
	METAL_BUFFER_TEXTURE_INDICES_INDEX = 4
} MetalBufferIndex;

typedef enum
{
	METAL_TEXTURE_INDEX = 0
} MetalTextureIndex;

#define METAL_MAX_TEXTURE_COUNT 16

#endif /* metal_indices_h */
