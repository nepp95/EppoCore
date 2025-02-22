#include "pch.h"
#include "Renderer.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Eppo
{
    Renderer::Renderer()
    {
        // Create uniform buffer
        m_UniformBuffer = std::make_unique<UniformBuffer>(sizeof(UboData), 0);

        // Compile shader
        m_Shader = std::make_unique<Shader>();

        // Setup index data
        const auto quadIndices = new uint32_t[MAX_INDICES];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < MAX_INDICES; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        // Setup OpenGL
        // Create vertex buffer
        glCreateBuffers(1, &m_VertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

        // Create index buffer
        glCreateBuffers(1, &m_IndexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_IndexBufferID);
        glBufferData(GL_ARRAY_BUFFER, MAX_INDICES * sizeof(uint32_t), quadIndices, GL_STATIC_DRAW);
        delete[] quadIndices;

        // Create vertex array
        glCreateVertexArrays(1, &m_VertexArrayID);
        glBindVertexArray(m_VertexArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);

        uint32_t index = 0;

        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
        index++;

        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(sizeof(glm::vec2)));

        glBindVertexArray(m_VertexArrayID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID);

        // Setup vertex data
        m_VertexBufferBase = new Vertex[MAX_VERTICES * sizeof(Vertex)];
        m_VertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        m_VertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        m_VertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
        m_VertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
    }

    Renderer::~Renderer()
    {
        glDeleteVertexArrays(1, &m_VertexArrayID);
        glDeleteBuffers(1, &m_VertexBufferID);
        glDeleteBuffers(1, &m_IndexBufferID);
    }

    void Renderer::BeginScene(const glm::mat4& projection)
    {
        m_UboData.ProjectionMatrix = projection;
        m_UniformBuffer->SetData(&m_UboData, sizeof(UboData));

        StartBatch();
    }

    void Renderer::EndScene() const
    {
        Flush();
    }

    void Renderer::DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position, 1.0f)) * glm::scale(
                glm::mat4(1.0f), glm::vec3(size, 1.0f));

        for (const auto& vertexPosition : m_VertexPositions)
        {
            m_VertexBufferPtr->Position = transform * vertexPosition;
            m_VertexBufferPtr->Color = color;
            m_VertexBufferPtr++;
        }

        m_IndexCount += 6;
    }

    void Renderer::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer::StartBatch()
    {
        m_VertexBufferPtr = m_VertexBufferBase;
        m_IndexCount = 0;
    }

    void Renderer::Flush() const
    {
        if (m_IndexCount == 0)
            return;

        // Update vertex buffer
        const auto dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(m_VertexBufferPtr) - reinterpret_cast<uint8_t*>(
            m_VertexBufferBase));
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_VertexBufferBase);

        // Draw call
        m_Shader->Bind();
        glBindVertexArray(m_VertexArrayID);
        glDrawElements(GL_TRIANGLES, static_cast<int>(m_IndexCount), GL_UNSIGNED_INT, nullptr);
    }
}
