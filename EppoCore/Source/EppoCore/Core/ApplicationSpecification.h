#pragma once

namespace Eppo
{
    struct ApplicationCommandLineArgs
    {
        ApplicationCommandLineArgs() = default;

        ApplicationCommandLineArgs(const int argc, char** argv)
            : Count(argc), Args(argv)
        {}

        int Count = 0;
        char** Args = nullptr;

        const char* operator[](const int index) const
        {
            EPPO_ASSERT((index < Count));
            if (index >= Count)
                return "";
            return Args[index];
        }
    };

    struct ApplicationSpecification
    {
        std::string Title{};
        uint32_t Width = 1280;
        uint32_t Height = 720;

        std::filesystem::path LocalWorkingDirectory = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();
        ApplicationCommandLineArgs CommandLineArgs;
    };
}
