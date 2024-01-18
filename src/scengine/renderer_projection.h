
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "renderer_types.h"
#include "platforms.h"

#define PROJECTION_FOV_ANGLE 45.0f // in degrees
#define PROJECTION_NEAR_VIEW_DISTANCE 0.01f
#define PROJECTION_FAR_VIEW_DISTANCE 10000.0f

ZGFloat computeProjectionAspectRatio(Renderer *renderer);

void updateGLProjectionMatrix(Renderer *renderer);

#if PLATFORM_APPLE
void updateMetalProjectionMatrix(Renderer *renderer);
#endif

#ifdef __cplusplus
}
#endif
