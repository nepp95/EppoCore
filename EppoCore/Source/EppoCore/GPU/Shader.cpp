#include "pch.h"
#include "Shader.h"

#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/ShaderCompiler.h"

namespace Eppo
{
    Shader::Shader(Device& device, const ShaderStage stage, const std::span<const uint32_t> spirv, const std::string_view name)
        : m_Device(&device), m_Stage(stage), m_Name(name)
    {
        Create(spirv);
    }

    Shader::Shader(Device& device, const std::filesystem::path& path)
        : m_Device(&device), m_Stage(ShaderCompiler::GetStage(path).value_or(ShaderStage::Compute)), m_Name(path.filename().string())
    {
        Create(ShaderCompiler::CompileFile(path));
    }

    Shader::Shader(
        Device& device, const ShaderStage stage, const std::string_view source, const std::string_view name,
        const std::filesystem::path& includeDirectory
    )
        : m_Device(&device), m_Stage(stage), m_Name(name)
    {
        Create(ShaderCompiler::Compile(stage, source, name, includeDirectory));
    }

    Shader::~Shader()
    {
        Release();
    }

    Shader::Shader(Shader&& other) noexcept
        : m_Device(other.m_Device),
          m_Stage(other.m_Stage),
          m_Name(std::move(other.m_Name)),
          m_Module(std::exchange(other.m_Module, VK_NULL_HANDLE)),
          m_Reflection(std::move(other.m_Reflection))
    {
    }

    auto Shader::operator=(Shader&& other) noexcept -> Shader&
    {
        if (this != &other)
        {
            Release();
            m_Device = other.m_Device;
            m_Stage = other.m_Stage;
            m_Name = std::move(other.m_Name);
            m_Module = std::exchange(other.m_Module, VK_NULL_HANDLE);
            m_Reflection = std::move(other.m_Reflection);
        }

        return *this;
    }

    auto Shader::Create(const std::span<const uint32_t> spirv) -> void
    {
        ZoneScoped;

        if (spirv.empty())
            return;

        auto reflection = ShaderReflection::Reflect(spirv, m_Stage, m_Name);
        if (!reflection)
            return;

        m_Reflection = std::move(*reflection);

        const VkShaderModuleCreateInfo moduleInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = spirv.size_bytes(),
            .pCode = spirv.data(),
        };

        EP_VK_CHECK(vkCreateShaderModule(m_Device->GetHandle(), &moduleInfo, nullptr, &m_Module));
        m_Device->SetDebugName(VK_OBJECT_TYPE_SHADER_MODULE, VkHandleToU64(m_Module), m_Name);
    }

    auto Shader::Release() -> void
    {
        if (!m_Module)
            return;

        // Pipelines don't reference their modules after creation, so the module can go right away
        vkDestroyShaderModule(m_Device->GetHandle(), m_Module, nullptr);
        m_Module = VK_NULL_HANDLE;
    }
}
