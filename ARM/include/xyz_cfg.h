#ifndef _xyz_cfg_h
#define _xyz_cfg_h

#include <sys/platform.h>

#include "umm_malloc.h"

/* Align FFT buffers on cache lines for efficient DMA */
#define XYZ_FFT_CALLOC(x,y)  umm_calloc_aligned(x,y,ADI_CACHE_LINE_LENGTH)
#define XYZ_FFT_FREE(x)      umm_free_aligned(x)

#endif
