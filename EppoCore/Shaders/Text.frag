#version 450 core

layout (location = 0) in vec2 inTexCoords;
layout (location = 1) in flat float inTexIndex;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D u_Text[32];
uniform vec3 u_TextColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_Text[int(inTexIndex)], inTexCoords).r);
    outColor = vec4(u_TextColor, 1.0) * sampled;
}