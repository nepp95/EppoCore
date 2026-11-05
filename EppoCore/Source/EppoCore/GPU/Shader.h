#pragma once

#include "EppoCore/GPU/ShaderReflection.h"
#include "EppoCore/GPU/ShaderStage.h"

namespace Eppo
{
    class Device;

    // A shader module with its reflection. Check IsValid(): compile/reflection errors are logged, not fatal.
    class Shader
    {
    public:
        // Precompiled SPIR-V, e.g. EppoCore's embedded shaders
        Shader(Device& device, ShaderStage stage, std::span<const uint32_t> spirv, std::string_view name = {});
        // GLSL file compiled at runtime; the stage comes from the extension (.vert, .frag, .comp)
        Shader(Device& device, const std::filesystem::path& path);
        // GLSL source compiled at runtime; includes resolve against includeDirectory
        Shader(
            Device& device, ShaderStage stage, std::string_view source, std::string_view name,
            const std::filesystem::path& includeDirectory = {}
        );
        ~Shader();

        Shader(const Shader&) = delete;
        auto operator=(const Shader&) -> Shader& = delete;
        Shader(Shader&& other) noexcept;
        auto operator=(Shader&& other) noexcept -> Shader&;

        [[nodiscard]] auto IsValid() const -> bool { return m_Module != VK_NULL_HANDLE; }
        [[nodiscard]] auto GetModule() const -> VkShaderModule { return m_Module; }
        [[nodiscard]] auto GetStage() const -> ShaderStage { return m_Stage; }
        [[nodiscard]] auto GetName() const -> const std::string& { return m_Name; }
        [[nodiscard]] auto GetReflection() const -> const ShaderReflection& { return m_Reflection; }

    private:
        auto Create(std::span<const uint32_t> spirv) -> void;
        auto Release() -> void;

    private:
        Device* m_Device = nullptr;
        ShaderStage m_Stage = ShaderStage::Compute;
        std::string m_Name;
        VkShaderModule m_Module = VK_NULL_HANDLE;
        ShaderReflection m_Reflection;
    };
}
