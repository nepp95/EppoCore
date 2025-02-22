#pragma once

#include "EppoCore/Core/Application.h"
#include "EppoCore/Core/Log.h"

int main(int argc, char** argv)
{
	Log::Init();

	const ApplicationCommandLineArgs args(argc, argv);

	const auto app = CreateApplication(args);
	app->Run();

	delete app;
	return 0;
}
