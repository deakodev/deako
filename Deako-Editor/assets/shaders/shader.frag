#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inTexCoord;

layout(location = 0) out vec4 outColor;

// layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 1) uniform sampler2DArray textureSampler;

void main()
{
    outColor = texture(textureSampler, inTexCoord) * vec4(inColor, 1.0);
}   

