#pragma once

#include <glm/glm.hpp>

namespace Eppo
{
    struct Vertex
    {
        glm::vec2 Position = glm::vec2(0.0f);
        glm::vec4 Color = glm::vec4(1.0f);

        Vertex() = default;

        Vertex(const glm::vec2& position, const glm::vec4& color)
            : Position(position), Color(color)
        {}
    };

    struct TextVertex
    {
        glm::vec2 Position = glm::vec2(0.0f);
        glm::vec2 TexCoords = glm::vec2(0.0f);
        float TexIndex = 0.0f;

        TextVertex() = default;

        TextVertex(const glm::vec2& position, const glm::vec2& texCoords)
            : Position(position), TexCoords(texCoords)
        {}
    };
}
