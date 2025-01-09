#pragma once

#ifdef EPPO_HEADLESS
	#include "EppoCore/Platform/Headless/ApplicationHeadless.h"
#else
	#include "EppoCore/Platform/GUI/ApplicationGUI.h"
#endif
