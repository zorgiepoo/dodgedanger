
#pragma once

#include "math_3d.h"
#include "renderer.h"

#define SUPPORT_DEPRECATED_DRAW_STRING_APIS 0

// Requires Font subsystem to be initialized first
void initText(Renderer *renderer, int textRenderingCacheCount);

#if SUPPORT_DEPRECATED_DRAW_STRING_APIS
// Deprecated
void drawStringf(Renderer *renderer, mat4_t modelViewMatrix, color4_t color, ZGFloat width, ZGFloat height, const char *format, ...);
// Deprecated
void drawString(Renderer *renderer, mat4_t modelViewMatrix, color4_t color, ZGFloat width, ZGFloat height, const char *string);
#endif

void drawStringScaled(Renderer *renderer, mat4_t modelViewMatrix, color4_t color, ZGFloat scale, const char *string);

void drawStringLeftAligned(Renderer *renderer, mat4_t modelViewMatrix, color4_t color, ZGFloat scale, const char *string);
