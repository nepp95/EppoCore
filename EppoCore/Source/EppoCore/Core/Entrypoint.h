#pragma once

#include "EppoCore/Core/Application.h"

int main(int argc, char** argv)
{
	Eppo::Log::Init();

	ApplicationCommandLineArgs args(argc, argv);

	auto app = Eppo::CreateApplication(args);
	app->Run();

	delete app;
	return 0;
}
