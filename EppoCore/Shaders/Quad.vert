#version 450 core

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_ProjectionMatrix;
};

void main()
{
    outColor = inColor;

    vec3 position = vec3(inPosition, 1.0);
    gl_Position = u_ProjectionMatrix * vec4(position, 1.0);
}