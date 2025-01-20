#pragma once

#include "EppoCore/Core/Application.h"
#include "EppoCore/Core/Log.h"

int main(int argc, char** argv)
{
	Eppo::Log::Init();

	const Eppo::ApplicationCommandLineArgs args(argc, argv);

	const auto app = Eppo::CreateApplication(args);
	app->Run();

	delete app;
	return 0;
}
