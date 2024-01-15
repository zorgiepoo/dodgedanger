
#pragma once

#include <stdint.h>

#define CONVERT_MS_TO_NS(x) ((uint64_t)(x * 1000000.0))

uint32_t ZGGetTicks(void);

uint64_t ZGGetNanoTicks(void);
