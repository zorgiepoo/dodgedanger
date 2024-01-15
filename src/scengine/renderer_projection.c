
#include "renderer_projection.h"
#include "math_3d.h"
#include <string.h>

ZGFloat computeProjectionAspectRatio(Renderer *renderer)
{
	// Some history is this used to be width / height (where both width and height were integers), resulting in 1 most of the time.
	ZGFloat sizeRatio = (ZGFloat)renderer->drawableWidth / (ZGFloat)renderer->drawableHeight;
	
	// If size is 4:3 or 4.3:1 just use 1.0
	if (fabsf(sizeRatio - (4.0f / 3.0f)) <= 0.01f || fabsf(sizeRatio - (4.3f / 3.0f)) <= 0.01f)
	{
		return (ZGFloat)1.0f;
	}
	else
	{
		return sizeRatio / (ZGFloat)(16.0f / 9.0f);
	}
}

static mat4_t computeGLProjectionMatrix(Renderer *renderer)
{
	return m4_perspective(PROJECTION_FOV_ANGLE, computeProjectionAspectRatio(renderer), PROJECTION_NEAR_VIEW_DISTANCE, PROJECTION_FAR_VIEW_DISTANCE);
}

void updateGLProjectionMatrix(Renderer *renderer)
{
	mat4_t glProjectionMatrix = computeGLProjectionMatrix(renderer);
	
	memcpy(renderer->projectionMatrix, glProjectionMatrix.m, sizeof(glProjectionMatrix.m));
}

#if PLATFORM_APPLE
void updateMetalProjectionMatrix(Renderer *renderer)
{
	mat4_t glProjectionMatrix = computeGLProjectionMatrix(renderer);
	
	// https://metashapes.com/blog/opengl-metal-projection-matrix-problem/
	mat4_t metalProjectionAdjustMatrix =
	mat4(
		 1.0f, 0.0f, 0.0f, 0.0f,
		 0.0f, 1.0f, 0.0f, 0.0f,
		 0.0f, 0.0f, 0.5f, 0.5f,
		 0.0f, 0.0f, 0.0f, 1.0f
		 );
	mat4_t finalProjectionMatrix = m4_mul(metalProjectionAdjustMatrix, glProjectionMatrix);
	
	memcpy(renderer->projectionMatrix, finalProjectionMatrix.m, sizeof(finalProjectionMatrix.m));
}
#endif
