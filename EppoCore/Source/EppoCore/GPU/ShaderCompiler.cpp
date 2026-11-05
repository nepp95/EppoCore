#include "pch.h"
#include "ShaderCompiler.h"

#include "EppoCore/Core/Filesystem.h"

#include <shaderc/shaderc.hpp>

namespace Eppo
{
    namespace
    {
        constexpr std::string_view s_BindlessIncludeName = "EppoCore/Bindless.glsl";

        // Declarations for the bindless table (descriptor set 1, see BindlessTable)
        constexpr std::string_view s_BindlessInclude = R"(#extension GL_EXT_nonuniform_qualifier : require

layout (set = 1, binding = 0) uniform texture2D u_BindlessImages[];
layout (set = 1, binding = 1) uniform sampler u_BindlessSamplers[];

#define EP_BINDLESS_SAMPLER2D(image, sampler) sampler2D(u_BindlessImages[nonuniformEXT(image)], u_BindlessSamplers[nonuniformEXT(sampler)])
)";

        class Includer final : public shaderc::CompileOptions::IncluderInterface
        {
        public:
            explicit Includer(std::filesystem::path includeDirectory)
                : m_IncludeDirectory(std::move(includeDirectory))
            {
            }

            auto GetInclude(const char* requested, const shaderc_include_type type, const char* requesting, size_t)
                -> shaderc_include_result* override
            {
                auto* result = new Result;

                if (type == shaderc_include_type_standard && requested == s_BindlessIncludeName)
                {
                    result->Name = s_BindlessIncludeName;
                    result->Content = s_BindlessInclude;
                }
                else
                {
                    const std::filesystem::path base =
                        type == shaderc_include_type_relative ? std::filesystem::path(requesting).parent_path() : m_IncludeDirectory;
                    const std::filesystem::path path = (base / requested).lexically_normal();

                    if (std::filesystem::exists(path))
                    {
                        result->Name = path.generic_string();
                        result->Content = ReadText(path);
                    }
                    else
                    {
                        // An empty name tells shaderc the include failed; the content is the error message
                        result->Content = fmt::format("Cannot find include file {}", path.generic_string());
                    }
                }

                result->Data = {
                    result->Name.c_str(), result->Name.size(), result->Content.c_str(), result->Content.size(), result,
                };

                return &result->Data;
            }

            auto ReleaseInclude(shaderc_include_result* data) -> void override { delete static_cast<Result*>(data->user_data); }

        private:
            struct Result
            {
                std::string Name;
                std::string Content;
                shaderc_include_result Data{};
            };

            std::filesystem::path m_IncludeDirectory;
        };

        auto ToShaderKind(const ShaderStage stage) -> shaderc_shader_kind
        {
            switch (stage)
            {
                case ShaderStage::Vertex:
                    return shaderc_glsl_vertex_shader;
                case ShaderStage::Fragment:
                    return shaderc_glsl_fragment_shader;
                case ShaderStage::Compute:
                    return shaderc_glsl_compute_shader;
            }

            return shaderc_glsl_infer_from_source;
        }
    }

    auto ShaderCompiler::Compile(
        const ShaderStage stage, const std::string_view source, const std::string_view name, const std::filesystem::path& includeDirectory
    ) -> std::vector<uint32_t>
    {
        ZoneScoped;

        shaderc::CompileOptions options;
        options.SetSourceLanguage(shaderc_source_language_glsl);
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetTargetSpirv(shaderc_spirv_version_1_6);
        options.SetIncluder(std::make_unique<Includer>(includeDirectory));

#ifdef EP_DEBUG
        options.SetOptimizationLevel(shaderc_optimization_level_zero);
        options.SetGenerateDebugInfo();
#else
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif

        // The input name is what relative includes in the root file resolve against
        const std::string inputName = (includeDirectory / name).generic_string();

        shaderc::Compiler compiler;
        const shaderc::SpvCompilationResult result =
            compiler.CompileGlslToSpv(source.data(), source.size(), ToShaderKind(stage), inputName.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            Log::Error("Compiling shader {} failed:\n{}", name, result.GetErrorMessage());
            return {};
        }

        if (result.GetNumWarnings() > 0)
            Log::Warn("Shader {}:\n{}", name, result.GetErrorMessage());

        return { result.cbegin(), result.cend() };
    }

    auto ShaderCompiler::CompileFile(const std::filesystem::path& path) -> std::vector<uint32_t>
    {
        const auto stage = GetStage(path);
        if (!stage)
        {
            Log::Error("Unknown shader stage for {}; use .vert, .frag or .comp", path);
            return {};
        }

        if (!std::filesystem::exists(path))
        {
            Log::Error("Shader file {} does not exist", path);
            return {};
        }

        return Compile(*stage, ReadText(path), path.filename().string(), path.parent_path());
    }

    auto ShaderCompiler::GetStage(const std::filesystem::path& path) -> std::optional<ShaderStage>
    {
        const std::string extension = path.extension().string();

        if (extension == ".vert")
            return ShaderStage::Vertex;
        if (extension == ".frag")
            return ShaderStage::Fragment;
        if (extension == ".comp")
            return ShaderStage::Compute;

        return std::nullopt;
    }
}
