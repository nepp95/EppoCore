#pragma once

#include <memory>

#define EXPAND_MACRO(x) x
#define STRINGIFY_MACRO(x) #x
#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#ifdef EPPO_WINDOWS
    #define DEBUGBREAK() __debugbreak()
#elif defined(EPPO_LINUX)
    #define DEBUGBREAK() asm("int $3")
#endif

#include "EppoCore/Core/Log.h"
#include "EppoCore/Core/Assert.h"
