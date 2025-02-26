#version 450 core

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in float inTexIndex;

layout (location = 0) out vec2 outTexCoords;
layout (location = 1) out flat float outTexIndex;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_ProjectionMatrix;
};

void main()
{
    outTexCoords = inTexCoord;
    outTexIndex = inTexIndex;

    gl_Position = u_ProjectionMatrix * vec4(inPos, 0.0, 1.0);
}