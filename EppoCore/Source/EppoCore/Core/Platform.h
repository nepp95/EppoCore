#pragma once

// Platform and configuration detection for consumers that don't get EppoCore's CMake defines (e.g. premake projects).
// CMake consumers already have these defined; the guards keep those values.

#if defined(_WIN32)
    #ifndef EP_PLATFORM_WINDOWS
        #define EP_PLATFORM_WINDOWS
    #endif
#elif defined(__linux__)
    #ifndef EP_PLATFORM_LINUX
        #define EP_PLATFORM_LINUX
    #endif
#else
    #error "EppoCore only supports Windows and Linux"
#endif

#if !defined(EP_DEBUG) && !defined(EP_RELEASE)
    #if defined(NDEBUG)
        #define EP_RELEASE
    #else
        #define EP_DEBUG
    #endif
#endif

// EP_GUI / EP_HEADLESS change class layouts (e.g. Layer's virtual functions), so the consumer must define the one
// matching the library it links: EP_GUI for EppoCore_GUI, EP_HEADLESS for EppoCore_Headless.
#if defined(EP_GUI) && defined(EP_HEADLESS)
    #error "Define only one of EP_GUI or EP_HEADLESS"
#elif !defined(EP_GUI) && !defined(EP_HEADLESS)
    #error "Define EP_GUI (when linking EppoCore_GUI) or EP_HEADLESS (when linking EppoCore_Headless)"
#endif
