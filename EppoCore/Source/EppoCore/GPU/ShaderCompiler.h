#pragma once

#include "EppoCore/GPU/ShaderStage.h"

namespace Eppo
{
    // GLSL -> SPIR-V (Vulkan 1.3) at runtime, for shaders that don't ship with EppoCore.
    // #include "file" resolves relative to the including file, #include <file> relative to the include directory.
    // <EppoCore/Bindless.glsl> is built in and declares the bindless table.
    class ShaderCompiler
    {
    public:
        // Empty on failure; errors are logged
        [[nodiscard]] static auto
        Compile(ShaderStage stage, std::string_view source, std::string_view name, const std::filesystem::path& includeDirectory = {})
            -> std::vector<uint32_t>;

        // Compiles a file; the stage comes from its extension (.vert, .frag, .comp)
        [[nodiscard]] static auto CompileFile(const std::filesystem::path& path) -> std::vector<uint32_t>;

        [[nodiscard]] static auto GetStage(const std::filesystem::path& path) -> std::optional<ShaderStage>;
    };
}
