#pragma once

#include "Shader.h"
#include "UniformBuffer.h"
#include "Vertex.h"

#include <glm/glm.hpp>

namespace Eppo
{
    constexpr uint32_t MAX_QUADS = 10000;
    constexpr uint32_t MAX_VERTICES = 4 * MAX_QUADS;
    constexpr uint32_t MAX_INDICES = 6 * MAX_QUADS;

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        void BeginScene(const glm::mat4& projection);
        void EndScene() const;

        void DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

    private:
        void NextBatch();
        void StartBatch();
        void Flush() const;

    private:
        std::unique_ptr<Shader> m_Shader;
        std::unique_ptr<UniformBuffer> m_UniformBuffer;

        struct UboData
        {
            glm::mat4 ProjectionMatrix;
        } m_UboData{};

        uint32_t m_VertexArrayID = 0;
        uint32_t m_VertexBufferID = 0;
        uint32_t m_IndexBufferID = 0;

        glm::vec4 m_VertexPositions[4]{};
        Vertex* m_VertexBufferBase = nullptr;
        Vertex* m_VertexBufferPtr = nullptr;

        uint32_t m_IndexCount = 0;
    };
}
