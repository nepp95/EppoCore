#include "pch.h"

// Todo: This is a workaround and should not stay since it disables simd optimization
#ifdef EPPO_LINUX
    #undef __OPTIMIZE__
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
