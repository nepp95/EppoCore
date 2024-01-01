#pragma once

#include <memory>

#define EXPAND_MACRO(x) x
#define STRINGIFY_MACRO(x) #x
#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#include "EppoCore/Core/Log.h"
#include "EppoCore/Core/Assert.h"