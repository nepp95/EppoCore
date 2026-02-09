#pragma once

#include "EppoCore/Core/Base.h"

#include <csignal>

#if defined(EP_DEBUG)
    #if defined(EP_PLATFORM_WINDOWS)
        #define EP_ASSERT(condition)                                                                                                       \
            if (!(condition))                                                                                                              \
                __debugbreak()
    #elif defined(EP_PLATFORM_LINUX)
        #define EP_ASSERT(condition)                                                                                                       \
            if (!(condition))                                                                                                              \
            raise(SIGTRAP)
    #else
        #define EP_ASSERT(condition)
    #endif
#else
    #define EP_ASSERT(condition)
#endif
