#pragma once

namespace Eppo
{
    class Shader
    {
    public:
        explicit Shader();
        ~Shader();

        void Bind() const;

        [[nodiscard]] uint32_t GetRendererID() const;

    private:
        std::string ReadFile(const std::filesystem::path& filepath);

    private:
        uint32_t m_RendererID = 0;
    };
}
