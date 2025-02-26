#pragma once

#include "Image.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "Vertex.h"

#include <glm/glm.hpp>

namespace Eppo
{
    constexpr uint32_t MAX_QUADS = 10000;
    constexpr uint32_t MAX_VERTICES = 4 * MAX_QUADS; // 960.000 bytes
    constexpr uint32_t MAX_INDICES = 6 * MAX_QUADS; // 240.000 bytes
    constexpr uint32_t MAX_TEXTURES = 32;

    struct Character
    {
        uint32_t RendererID; // Handle of the glyph texture
        uint32_t Advance; // Offset to advance to next glyph
        glm::ivec2 Size; // Size of glyph
        glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
    };

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        void BeginScene(const glm::mat4& projection);
        void EndScene() const;

        void DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        void DrawString(const glm::vec2& position, const std::string& text, float fontSize = 12.0f);

    private:
        void NextBatch();
        void StartBatch();
        void Flush() const;

    private:
        // Uniforms
        struct UboData
        {
            glm::mat4 ProjectionMatrix;
        } m_UboData{};

        std::unique_ptr<UniformBuffer> m_UniformBuffer;

        // Shaders
        std::unique_ptr<Shader> m_QuadShader;
        std::unique_ptr<Shader> m_TextShader;

        // Buffers
        uint32_t m_QuadVertexArrayID = 0;
        uint32_t m_QuadVertexBufferID = 0;
        uint32_t m_QuadIndexBufferID = 0;
        uint32_t m_TextVertexArrayID = 0;
        uint32_t m_TextVertexBufferID = 0;
        uint32_t m_TextIndexBufferID = 0;

        Vertex* m_QuadVertexBufferBase = nullptr;
        Vertex* m_QuadVertexBufferPtr = nullptr;
        TextVertex* m_TextVertexBufferBase = nullptr;
        TextVertex* m_TextVertexBufferPtr = nullptr;

        // Other data
        glm::vec4 m_QuadVertexPositions[4]{};
        uint32_t m_QuadIndexCount = 0;
        uint32_t m_TextIndexCount = 0;

        std::array<uint32_t, MAX_TEXTURES> m_Textures{};
        uint32_t m_TextureIndex = 0;

        std::map<unsigned char, Character> m_Characters;
    };
}
