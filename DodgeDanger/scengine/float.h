
#pragma once

#define USE_HALF_WHEN_POSSIBLE 0

#if defined(__METAL_VERSION__)

#if USE_HALF_WHEN_POSSIBLE
typedef half4 ZGFloat4;
typedef half2 ZGFloat2;
typedef half ZGFloat;
typedef matrix_half4x4 ZGFloat_matrix4x4;
#else
typedef float4 ZGFloat4;
typedef float2 ZGFloat2;
typedef float ZGFloat;
typedef matrix_float4x4 ZGFloat_matrix4x4;
#endif

#else

#include "platforms.h"

#if PLATFORM_APPLE

#if USE_HALF_WHEN_POSSIBLE
typedef _Float16 ZGFloat;
#else
typedef float ZGFloat;
#endif

#else
typedef float ZGFloat;
#endif


#endif
