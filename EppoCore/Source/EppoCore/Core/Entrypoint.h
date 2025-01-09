#pragma once

#include "EppoCore/Core/Application.h"

int main(int argc, char** argv)
{
	Log::Init();

	const ApplicationCommandLineArgs args(argc, argv);

	const auto app = Eppo::CreateApplication(args);
	app->Run();

	delete app;
	return 0;
}
