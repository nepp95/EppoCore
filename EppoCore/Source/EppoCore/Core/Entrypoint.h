#pragma once

#include "EppoCore/Core/Application.h"

int main(int argc, char** argv)
{
	Eppo::Log::Init();

	auto app = Eppo::CreateApplication();
	app->Run();

	delete app;
	return 0;
}
